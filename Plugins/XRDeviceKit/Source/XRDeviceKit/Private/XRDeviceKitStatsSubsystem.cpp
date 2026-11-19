// 资源采样器的实现：读 /proc 拿进程和整机数据，读引擎统计拿帧时间和内存。
// 在 Win64 上 /proc 不存在，相关字段保持 0；帧时间、引擎内存照样有值。

#include "XRDeviceKitStatsSubsystem.h"
#include "XRDeviceKitPlatform.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMemory.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

DEFINE_LOG_CATEGORY_STATIC(LogXRDeviceKitStats, Log, All);

// 引擎平滑帧率，StatUnitData 没值时用它反推帧耗时
extern ENGINE_API float GAverageFPS;

namespace
{
// Android 的 USER_HZ 固定是 100：/proc 里的 jiffies 除以它才是秒
constexpr double kJiffiesPerSecond = 100.0;

// /proc 文件不能用 FFileHelper 读：它们 stat 出来大小是 0（有些是 -1），
// FFileHelper 会按这个大小分配缓冲区，拿垃圾长度去 memset 直接段错误。
// 所以这里用 fopen/fread 定长读，读满缓冲或遇到 EOF 就停。
FString ReadProcFile(const TCHAR* Path)
{
	// Windows 用 fopen_s 避开 C4996 警告；其它平台（/proc 只存在于这些平台）用 fopen
	FILE* File = nullptr;
#if PLATFORM_WINDOWS
	fopen_s(&File, TCHAR_TO_UTF8(Path), "r");
#else
	File = fopen(TCHAR_TO_UTF8(Path), "r");
#endif
	if (File == nullptr)
	{
		return FString();
	}

	char Buffer[8192];
	size_t Total = 0;
	while (Total < sizeof(Buffer) - 1)
	{
		const size_t ReadThisTime = fread(Buffer + Total, 1, sizeof(Buffer) - 1 - Total, File);
		if (ReadThisTime == 0)
		{
			break;
		}
		Total += ReadThisTime;
	}
	fclose(File);

	Buffer[Total] = '\0';
	return FString(UTF8_TO_TCHAR(Buffer));
}

// 从 /proc/self/status 这种 "Key:   1234 kB" 的行里取数值（单位 kB），取不到返回 false
bool ReadStatusKilobytes(const FString& Content, const TCHAR* Key, int64& OutKilobytes)
{
	TArray<FString> Lines;
	Content.ParseIntoArrayLines(Lines);
	for (const FString& Line : Lines)
	{
		if (!Line.StartsWith(Key))
		{
			continue;
		}
		FString Value = Line.RightChop(FCString::Strlen(Key));
		Value.ReplaceInline(TEXT("kB"), TEXT(""));
		Value.TrimStartAndEndInline();
		OutKilobytes = FCString::Strtoui64(*Value, nullptr, 10);
		return true;
	}
	return false;
}

// 从 /proc/meminfo 取 "MemAvailable:   1234 kB"，取不到返回 false
bool ReadMeminfoKilobytes(const FString& Content, const TCHAR* Key, int64& OutKilobytes)
{
	return ReadStatusKilobytes(Content, Key, OutKilobytes);
}

// /proc/self/stat：字段是 "pid (comm) state ..."，comm 里可能有空格和括号，
// 所以从最后一个 ')' 之后开始数：第 1 个是 state，utime 和 stime 是第 12、13 个。
bool ReadProcessCpuTicks(uint64& OutTicks)
{
	const FString Content = ReadProcFile(TEXT("/proc/self/stat"));
	if (Content.IsEmpty())
	{
		return false;
	}

	const int32 CloseParen = Content.Find(TEXT(")"), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	if (CloseParen == INDEX_NONE)
	{
		return false;
	}

	TArray<FString> Fields;
	Content.RightChop(CloseParen + 1).TrimStartAndEnd().ParseIntoArrayWS(Fields);
	if (Fields.Num() < 13)
	{
		return false;
	}

	const uint64 UserTicks = FCString::Strtoui64(*Fields[11], nullptr, 10);
	const uint64 SystemTicks = FCString::Strtoui64(*Fields[12], nullptr, 10);
	OutTicks = UserTicks + SystemTicks;
	return true;
}

// /proc/stat 第一行：cpu  user nice system idle iowait irq softirq steal ...
// 总时间取前 8 项之和，空闲取 idle + iowait
bool ReadSystemCpuTicks(uint64& OutTotalTicks, uint64& OutIdleTicks)
{
	const FString Content = ReadProcFile(TEXT("/proc/stat"));
	if (Content.IsEmpty())
	{
		return false;
	}

	TArray<FString> Lines;
	Content.ParseIntoArrayLines(Lines);
	if (Lines.Num() == 0 || !Lines[0].StartsWith(TEXT("cpu")))
	{
		return false;
	}

	TArray<FString> Fields;
	Lines[0].ParseIntoArrayWS(Fields);
	if (Fields.Num() < 9)
	{
		return false;
	}

	uint64 Total = 0;
	for (int32 Index = 1; Index <= 8; ++Index)
	{
		Total += FCString::Strtoui64(*Fields[Index], nullptr, 10);
	}
	// Fields[4] 是 idle，Fields[5] 是 iowait
	OutIdleTicks = FCString::Strtoui64(*Fields[4], nullptr, 10) + FCString::Strtoui64(*Fields[5], nullptr, 10);
	OutTotalTicks = Total;
	return true;
}

// /proc/uptime 兜底：Android 12 上 /proc/stat 常被 SELinux 挡住读不到，
// 但 /proc/uptime 可以读：第一列是开机秒数，第二列是所有 CPU 的空闲秒数之和。
bool ReadUptimeSeconds(double& OutUptimeSec, double& OutIdleSec)
{
	const FString Content = ReadProcFile(TEXT("/proc/uptime"));
	if (Content.IsEmpty())
	{
		return false;
	}

	TArray<FString> Fields;
	Content.ParseIntoArrayWS(Fields);
	if (Fields.Num() < 2)
	{
		return false;
	}

	OutUptimeSec = FCString::Atod(*Fields[0]);
	OutIdleSec = FCString::Atod(*Fields[1]);
	return true;
}

// /proc/self/io 的 read_bytes / write_bytes，单位字节
void ReadProcessIo(uint64& OutReadBytes, uint64& OutWriteBytes)
{
	const FString Content = ReadProcFile(TEXT("/proc/self/io"));
	if (Content.IsEmpty())
	{
		return;
	}

	TArray<FString> Lines;
	Content.ParseIntoArrayLines(Lines);
	for (const FString& Line : Lines)
	{
		FString Text = Line;
		if (Text.StartsWith(TEXT("read_bytes:")))
		{
			Text.RightChop(FCString::Strlen(TEXT("read_bytes:"))).TrimStartAndEndInline();
			OutReadBytes = FCString::Strtoui64(*Text, nullptr, 10);
		}
		else if (Text.StartsWith(TEXT("write_bytes:")))
		{
			Text.RightChop(FCString::Strlen(TEXT("write_bytes:"))).TrimStartAndEndInline();
			OutWriteBytes = FCString::Strtoui64(*Text, nullptr, 10);
		}
	}
}
} // namespace

void UXRDeviceKitStatsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 采样间隔允许用 ini 覆盖，默认 1 秒
	GConfig->GetFloat(TEXT("XRDeviceKit"), TEXT("StatsInterval"), SampleInterval, GGameIni);
	SampleInterval = FMath::Clamp(SampleInterval, 0.2f, 10.0f);

	// 常驻采样：1 秒一次，喂 GetSnapshot；打不打日志由 StartLogging 决定
	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UXRDeviceKitStatsSubsystem::OnTick), 0.0f);

	UE_LOG(LogXRDeviceKitStats, Log, TEXT("资源采样器已启动，间隔 %.2f 秒"), SampleInterval);
}

void UXRDeviceKitStatsSubsystem::Deinitialize()
{
	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}
	Super::Deinitialize();
}

bool UXRDeviceKitStatsSubsystem::OnTick(float DeltaSeconds)
{
	TimeSinceSample += DeltaSeconds;
	if (TimeSinceSample < SampleInterval)
	{
		return true;
	}
	TimeSinceSample = 0.0f;

	TakeSample();
	if (bLogging)
	{
		LogSnapshot(LastSnapshot);
	}
	return true;
}

void UXRDeviceKitStatsSubsystem::StartLogging(float IntervalSeconds)
{
	SampleInterval = FMath::Clamp(IntervalSeconds > 0.0f ? IntervalSeconds : 1.0f, 0.2f, 10.0f);
	bLogging = true;
	TimeSinceSample = SampleInterval;                                               // 下一次 tick 立刻打一条，不用等一个间隔
	UE_LOG(LogXRDeviceKitStats, Log, TEXT("开始输出资源统计，间隔 %.2f 秒"), SampleInterval);
}

void UXRDeviceKitStatsSubsystem::StopLogging()
{
	bLogging = false;
	UE_LOG(LogXRDeviceKitStats, Log, TEXT("停止输出资源统计"));
}

void UXRDeviceKitStatsSubsystem::TakeSample()
{
	FXRDeviceResourceSnapshot Snapshot;
	const double Now = FPlatformTime::Seconds();

	// ---- 引擎侧：帧时间、线程耗时、GPU ----
	if (GEngine != nullptr && GEngine->GameViewport != nullptr)
	{
		if (const FStatUnitData* Unit = GEngine->GameViewport->GetStatUnitData())
		{
			Snapshot.FrameTimeMs = Unit->FrameTime;
			Snapshot.GameThreadMs = Unit->GameThreadTime;
			Snapshot.RenderThreadMs = Unit->RenderThreadTime;
			Snapshot.RhitMs = Unit->RHITTime;
			Snapshot.GpuFrameTimeMs = Unit->GPUFrameTime[0];
			Snapshot.GpuUsageFraction = Unit->GPUUsageFraction[0];
		}
	}

	// StatUnitData 只在引擎内部刷新统计时才有值，拿不到就用引擎平滑帧率反推
	if (Snapshot.FrameTimeMs <= 0.0f && GAverageFPS > 0.0f)
	{
		Snapshot.FrameTimeMs = 1000.0f / GAverageFPS;
	}

	// ---- 引擎内存 ----
	const FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
	Snapshot.EngineMemoryUsedMB = static_cast<int64>(MemoryStats.UsedPhysical / (1024 * 1024));
	Snapshot.SystemAvailableMB = static_cast<int64>(MemoryStats.AvailablePhysical / (1024 * 1024));

	// ---- 电池：复用已有节点 ----
	Snapshot.BatteryCurrentMa = XRDeviceKit::Platform::GetBatteryCurrentNow();
	Snapshot.BatteryTempC = XRDeviceKit::Platform::GetBatteryTemperature();

#if PLATFORM_ANDROID
	// ---- 进程内存与线程数 ----
	const FString Status = ReadProcFile(TEXT("/proc/self/status"));
	int64 KilobyteValue = 0;
	if (ReadStatusKilobytes(Status, TEXT("VmRSS:"), KilobyteValue))
	{
		Snapshot.MemoryUsedMB = KilobyteValue / 1024;
	}
	if (ReadStatusKilobytes(Status, TEXT("VmHWM:"), KilobyteValue))
	{
		Snapshot.MemoryPeakMB = KilobyteValue / 1024;
	}
	if (ReadStatusKilobytes(Status, TEXT("Threads:"), KilobyteValue))
	{
		Snapshot.ThreadCount = static_cast<int32>(KilobyteValue);
	}

	// ---- 整机可用内存 ----
	const FString Meminfo = ReadProcFile(TEXT("/proc/meminfo"));
	if (ReadMeminfoKilobytes(Meminfo, TEXT("MemAvailable:"), KilobyteValue))
	{
		Snapshot.SystemAvailableMB = KilobyteValue / 1024;
	}

	// ---- CPU 占用：两次采样求差 ----
	uint64 ProcessTicks = 0;
	uint64 SystemTotalTicks = 0;
	uint64 SystemIdleTicks = 0;
	const bool bHaveProcess = ReadProcessCpuTicks(ProcessTicks);
	bool bHaveSystem = ReadSystemCpuTicks(SystemTotalTicks, SystemIdleTicks);

	// /proc/stat 读不到（SELinux）就走 /proc/uptime 兜底
	double UptimeSec = 0.0;
	double UptimeIdleSec = 0.0;
	const bool bHaveUptime = !bHaveSystem && ReadUptimeSeconds(UptimeSec, UptimeIdleSec);
	bHaveSystem = bHaveSystem || bHaveUptime;

	if (bHasPrevious && Now > PrevSampleTime)
	{
		const double DeltaSeconds = Now - PrevSampleTime;

		// 本进程占整机的百分比：进程 CPU 秒 / 实际经过秒 * 100。
		// 注意这里不依赖整机数据：/proc/stat 被 SELinux 挡住时进程 CPU 照样要能算。
		if (bHaveProcess)
		{
			const uint64 DeltaProcess = ProcessTicks - PrevProcessTicks;
			Snapshot.CpuPercent = static_cast<float>((DeltaProcess / kJiffiesPerSecond) / DeltaSeconds * 100.0);
		}

		// 整机占用：非空闲时间 / 总时间 * 100
		if (bHaveUptime)
		{
			// 多核：总时间 = 经过秒数 * 核心数，空闲是所有核的空闲之和
			const double Cores = FMath::Max(1, FPlatformMisc::NumberOfCores());
			const double TotalSeconds = (UptimeSec - PrevUptimeSec) * Cores;
			const double IdleSeconds = UptimeIdleSec - PrevUptimeIdleSec;
			if (TotalSeconds > 0.0)
			{
				const double Busy = FMath::Clamp((TotalSeconds - IdleSeconds) / TotalSeconds, 0.0, 1.0);
				Snapshot.SystemCpuPercent = static_cast<float>(Busy * 100.0);
			}
		}
		else if (bHaveSystem)
		{
			const uint64 DeltaTotal = SystemTotalTicks - PrevSystemTotalTicks;
			const uint64 DeltaIdle = SystemIdleTicks - PrevSystemIdleTicks;
			if (DeltaTotal > 0)
			{
				Snapshot.SystemCpuPercent = static_cast<float>((DeltaTotal - DeltaIdle) * 100.0 / DeltaTotal);
			}
		}
	}

	PrevProcessTicks = ProcessTicks;
	PrevSystemTotalTicks = SystemTotalTicks;
	PrevSystemIdleTicks = SystemIdleTicks;
	PrevUptimeSec = UptimeSec;
	PrevUptimeIdleSec = UptimeIdleSec;
	PrevSampleTime = Now;
	bHasPrevious = true;

	// ---- IO ----
	uint64 ReadBytes = 0;
	uint64 WriteBytes = 0;
	ReadProcessIo(ReadBytes, WriteBytes);
	Snapshot.IoReadMB = static_cast<float>(ReadBytes / (1024.0 * 1024.0));
	Snapshot.IoWriteMB = static_cast<float>(WriteBytes / (1024.0 * 1024.0));
#endif

	LastSnapshot = Snapshot;
}

void UXRDeviceKitStatsSubsystem::LogSnapshot(const FXRDeviceResourceSnapshot& Snapshot) const
{
	// 一行一个采样，前缀固定 STATS|，方便 adb 或日志文件里筛
	UE_LOG(LogXRDeviceKitStats, Display,
		TEXT("STATS|CPU=%.1f%% SYS=%.1f%% MEM=%lldMB PEAK=%lldMB ENG=%lldMB AVAIL=%lldMB THREADS=%d FPS=%.1f FRAME=%.1fms GT=%.1f RT=%.1f RHI=%.1f GPU=%.1fms GPUSE=%.2f IO=%.1f/%.1fMB THERMAL=%d BATT=%dmA %.1fC"),
		Snapshot.CpuPercent,
		Snapshot.SystemCpuPercent,
		Snapshot.MemoryUsedMB,
		Snapshot.MemoryPeakMB,
		Snapshot.EngineMemoryUsedMB,
		Snapshot.SystemAvailableMB,
		Snapshot.ThreadCount,
	 Snapshot.FrameTimeMs > 0.0f ? (1000.0f / Snapshot.FrameTimeMs) : 0.0f,
		Snapshot.FrameTimeMs,
		Snapshot.GameThreadMs,
		Snapshot.RenderThreadMs,
		Snapshot.RhitMs,
		Snapshot.GpuFrameTimeMs,
		Snapshot.GpuUsageFraction,
		Snapshot.IoReadMB,
		Snapshot.IoWriteMB,
		Snapshot.ThermalStatus,
		Snapshot.BatteryCurrentMa,
		Snapshot.BatteryTempC);
}
