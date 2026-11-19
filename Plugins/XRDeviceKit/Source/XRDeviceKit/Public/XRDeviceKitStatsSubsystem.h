// 资源采样器：每 N 秒读一次 /proc 和引擎统计，缓存最近一次快照供蓝图读取。
// 采样必须成对做——CPU 占用是两次采样的差值，单点查询没有意义，所以做成后台定时器。

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "XRDeviceKitStatsTypes.h"
#include "XRDeviceKitStatsSubsystem.generated.h"

UCLASS()
class XRDEVICEKIT_API UXRDeviceKitStatsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 最近一次采样。还没采过时返回全 0 的默认值。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Stats")
	FXRDeviceResourceSnapshot GetSnapshot() const { return LastSnapshot; }

	// 开始把每次采样打进 UE 日志。IntervalSeconds <= 0 时用默认 1 秒。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Stats")
	void StartLogging(float IntervalSeconds = 1.0f);

	// 停止打日志。采样本身不受影响。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Stats")
	void StopLogging();

	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Stats")
	bool IsLogging() const { return bLogging; }

private:
	bool OnTick(float DeltaSeconds);
	void TakeSample();
	void LogSnapshot(const FXRDeviceResourceSnapshot& Snapshot) const;

	FXRDeviceResourceSnapshot LastSnapshot;                                         // 最近一次采样结果
	bool bLogging = false;                                                          // 是否把采样打进日志
	float SampleInterval = 1.0f;                                                    // 采样间隔（秒）
	float TimeSinceSample = 0.0f;                                                   // 距上次采样的累计时间
	FTSTicker::FDelegateHandle TickerHandle;

	// 上一次的计数，用来算差值；第一次采样只记录不算
	uint64 PrevProcessTicks = 0;                                                    // /proc/self/stat 的 utime+stime
	uint64 PrevSystemTotalTicks = 0;                                                // /proc/stat 的总 jiffies
	uint64 PrevSystemIdleTicks = 0;                                                 // /proc/stat 的空闲 jiffies
	double PrevSampleTime = 0.0;                                                    // 上次采样时刻（秒）
	double PrevUptimeSec = 0.0;                                                     // /proc/uptime 第一列
	double PrevUptimeIdleSec = 0.0;                                                 // /proc/uptime 第二列（所有核空闲之和）
	bool bHasPrevious = false;                                                      // 是否已经有上一次的数据
};
