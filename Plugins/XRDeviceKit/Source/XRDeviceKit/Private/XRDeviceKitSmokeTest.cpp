// 真机冒烟测试的实现：把 56 个节点全调一遍，每个打一行 SMOKE|名字=值。
// 用固定前缀是为了在 logcat 里能直接 findstr "SMOKE|"。

#include "XRDeviceKitSmokeTest.h"
#include "XRDeviceKitLibrary.h"
#include "XRDeviceKitTypes.h"
#include "Containers/Ticker.h"
#include "Misc/ConfigCacheIni.h"

DEFINE_LOG_CATEGORY_STATIC(LogXRDeviceKitSmoke, Log, All);

namespace
{
// 一行一个值，前缀固定，方便脚本或 findstr 抓
#define SMOKE_I(Label, Value) UE_LOG(LogXRDeviceKitSmoke, Display, TEXT("SMOKE|%s=%d"), TEXT(Label), (int32)(Value))
#define SMOKE_L(Label, Value) UE_LOG(LogXRDeviceKitSmoke, Display, TEXT("SMOKE|%s=%lld"), TEXT(Label), (long long)(Value))
#define SMOKE_F(Label, Value) UE_LOG(LogXRDeviceKitSmoke, Display, TEXT("SMOKE|%s=%.3f"), TEXT(Label), (double)(Value))
#define SMOKE_B(Label, Value) UE_LOG(LogXRDeviceKitSmoke, Display, TEXT("SMOKE|%s=%s"), TEXT(Label), (Value) ? TEXT("true") : TEXT("false"))
#define SMOKE_S(Label, Value) UE_LOG(LogXRDeviceKitSmoke, Display, TEXT("SMOKE|%s=%s"), TEXT(Label), *(Value))
}

void UXRDeviceKitSmokeTestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	bool bRunOnStartup = false;
	GConfig->GetBool(TEXT("XRDeviceKit"), TEXT("SmokeTestOnStartup"), bRunOnStartup, GGameIni);
	if (!bRunOnStartup)
	{
		return;
	}

	UE_LOG(LogXRDeviceKitSmoke, Display, TEXT("SMOKE|__scheduled=1"));

	// 推迟 3 秒：等 Java 侧 Init 完成、场景起来，免得拿到一堆空值
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float)
	{
		RunSmokeTest();
		return false;
	}), 3.0f);
}

void UXRDeviceKitSmokeTestSubsystem::RunSmokeTest()
{
	using L = UXRDeviceKitLibrary;

	UE_LOG(LogXRDeviceKitSmoke, Display, TEXT("SMOKE|__begin=1"));

	// ---- 设备 / 电源 ----
	SMOKE_I("GetBatteryLevel", L::GetBatteryLevel());
	SMOKE_B("IsCharging", L::IsCharging());
	SMOKE_B("IsLowPowerMode", L::IsLowPowerMode());
	SMOKE_F("GetBatteryTemperature", L::GetBatteryTemperature());
	SMOKE_F("GetCPUTemperature", L::GetCPUTemperature());
	SMOKE_S("GetDeviceId", L::GetDeviceId());
	SMOKE_S("GetOSVersionString", L::GetOSVersionString());
	SMOKE_I("GetAndroidMajorVersion", L::GetAndroidMajorVersion());
	SMOKE_S("GetCPUBrand", L::GetCPUBrand());
	SMOKE_I("GetCoreCount", L::GetCoreCount());
	SMOKE_S("GetDefaultLocale", L::GetDefaultLocale());
	SMOKE_L("GetDiskFreeMegabytes", L::GetDiskFreeMegabytes(TEXT("/sdcard")));
	SMOKE_L("GetDiskTotalMegabytes", L::GetDiskTotalMegabytes(TEXT("/sdcard")));

	// ---- 设备构建信息 ----
	SMOKE_S("GetManufacturer", L::GetManufacturer());
	SMOKE_S("GetModel", L::GetModel());
	SMOKE_S("GetDeviceName", L::GetDeviceName());
	SMOKE_S("GetSupportedABIs", L::GetSupportedABIs());

	// ---- 显示 ----
	SMOKE_B("IsScreenOn", L::IsScreenOn());
	SMOKE_F("GetBrightness", L::GetBrightness());
	SMOKE_I("GetScreenOrientation", (int32)L::GetScreenOrientation());
	SMOKE_I("GetScreenWidth", L::GetScreenWidth());
	SMOKE_I("GetScreenHeight", L::GetScreenHeight());
	SMOKE_I("GetScreenDpi", L::GetScreenDpi());
	SMOKE_F("GetScreenDensity", L::GetScreenDensity());
	SMOKE_F("GetScreenRefreshRate", L::GetScreenRefreshRate());
	SMOKE_I("GetSystemBrightness", L::GetSystemBrightness());
	SMOKE_B("IsNightMode", L::IsNightMode());

	// ---- 网络 ----
	SMOKE_I("GetNetworkType", (int32)L::GetNetworkType());
	SMOKE_B("HasWiFiConnection", L::HasWiFiConnection());

	// ---- 音频 ----
	SMOKE_I("GetDeviceVolume", L::GetDeviceVolume());
	SMOKE_I("GetMaxVolume", L::GetMaxVolume());
	SMOKE_B("IsHeadphonesPluggedIn", L::IsHeadphonesPluggedIn());

	// ---- 存储 ----
	SMOKE_L("GetInternalFreeMegabytes", L::GetInternalFreeMegabytes());
	SMOKE_L("GetInternalTotalMegabytes", L::GetInternalTotalMegabytes());
	SMOKE_L("GetExternalFreeMegabytes", L::GetExternalFreeMegabytes());
	SMOKE_L("GetExternalTotalMegabytes", L::GetExternalTotalMegabytes());

	// ---- 应用 ----
	SMOKE_S("GetPackageName", L::GetPackageName());
	SMOKE_S("GetVersionName", L::GetVersionName());
	SMOKE_L("GetVersionCode", L::GetVersionCode());
	SMOKE_S("GetAppLabel", L::GetAppLabel());
	SMOKE_L("GetFirstInstallTimeMs", L::GetFirstInstallTimeMs());
	SMOKE_L("GetLastUpdateTimeMs", L::GetLastUpdateTimeMs());

	// ---- 系统 ----
	SMOKE_L("GetUptimeSeconds", L::GetUptimeSeconds());
	SMOKE_S("GetTimeZoneId", L::GetTimeZoneId());
	SMOKE_S("GetLanguageTag", L::GetLanguageTag());
	SMOKE_S("GetProjectVersion", L::GetProjectVersion());

	// 剪贴板：写进去再读出来，验证往返一致
	L::CopyToClipboard(TEXT("XRDeviceKitSmoke"));
	SMOKE_S("PasteFromClipboard", L::PasteFromClipboard());

	UE_LOG(LogXRDeviceKitSmoke, Display, TEXT("SMOKE|__end=1"));
}
