// 平台层的唯一实现文件。
// 这里是全插件唯一出现 #if PLATFORM_ANDROID 的地方：上面是真实现，下面是给编辑器
// （Win64 / Mac）用的假数据，保证没有安卓设备时工程也能正常打开和调试。

#include "XRDeviceKitPlatform.h"
#include "XRDeviceKitJni.h"
#include "XRDeviceKitJava.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/App.h"

DEFINE_LOG_CATEGORY_STATIC(LogXRDeviceKit, Log, All);

namespace
{
// 字节换成 MB，除不尽向下取整，展示用够了
int64 BytesToMegabytes(uint64 Bytes)
{
	return static_cast<int64>(Bytes / (1024ULL * 1024ULL));
}
} // namespace

// ==================== 设备 / 电源 ====================

int32 XRDeviceKit::Platform::GetBatteryLevel()
{
	// 引擎自带接口：Android 读系统广播拿到 0~100，其他平台内部直接返回 -1，不用分支
	return FPlatformMisc::GetBatteryLevel();
}

bool XRDeviceKit::Platform::IsCharging()
{
#if PLATFORM_ANDROID
	const FAndroidMisc::FBatteryState BatteryState = FAndroidMisc::GetBatteryState();
	return BatteryState.State == FAndroidMisc::BATTERY_STATE_CHARGING || BatteryState.State == FAndroidMisc::BATTERY_STATE_FULL;
#else
	return false;
#endif
}

bool XRDeviceKit::Platform::IsLowPowerMode()
{
	return FPlatformMisc::IsInLowPowerMode();
}

float XRDeviceKit::Platform::GetBatteryTemperature()
{
#if PLATFORM_ANDROID
	return FPlatformMisc::GetDeviceTemperatureLevel();
#else
	return -1.0f;
#endif
}

float XRDeviceKit::Platform::GetCPUTemperature()
{
#if PLATFORM_ANDROID
	return FPlatformMisc::GetCPUTemperature();
#else
	return -1.0f;
#endif
}

FString XRDeviceKit::Platform::GetDeviceId()
{
	return FPlatformMisc::GetDeviceId();
}

FString XRDeviceKit::Platform::GetOSVersionString()
{
	return FPlatformMisc::GetOSVersion();
}

int32 XRDeviceKit::Platform::GetAndroidMajorVersion()
{
#if PLATFORM_ANDROID
	return FAndroidMisc::GetAndroidMajorVersion();
#else
	return -1;
#endif
}

FString XRDeviceKit::Platform::GetCPUBrand()
{
#if PLATFORM_ANDROID
	return FPlatformMisc::GetCPUBrand();
#else
	return FString();
#endif
}

int32 XRDeviceKit::Platform::GetCoreCount()
{
	return FPlatformMisc::NumberOfCores();
}

FString XRDeviceKit::Platform::GetDefaultLocale()
{
	return FPlatformMisc::GetDefaultLocale();
}

int64 XRDeviceKit::Platform::GetDiskFreeMegabytes(const FString& Path)
{
	uint64 TotalBytes = 0;
	uint64 FreeBytes = 0;
	// 查询失败时引擎返回 false，两个出参都保持 0
	if (!FPlatformMisc::GetDiskTotalAndFreeSpace(Path, TotalBytes, FreeBytes))
	{
		return -1;
	}
	return BytesToMegabytes(FreeBytes);
}

int64 XRDeviceKit::Platform::GetDiskTotalMegabytes(const FString& Path)
{
	uint64 TotalBytes = 0;
	uint64 FreeBytes = 0;
	if (!FPlatformMisc::GetDiskTotalAndFreeSpace(Path, TotalBytes, FreeBytes))
	{
		return -1;
	}
	return BytesToMegabytes(TotalBytes);
}

// ==================== 显示 ====================

bool XRDeviceKit::Platform::IsScreenOn()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Jni::IsScreenOn();
#else
	// 编辑器里没有真实屏幕状态，当成亮着，免得调试界面一直走"熄屏"分支
	return true;
#endif
}

float XRDeviceKit::Platform::GetBrightness()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Jni::GetBrightness();
#else
	// 非安卓平台没有窗口亮度概念，返回满亮度
	return 1.0f;
#endif
}

bool XRDeviceKit::Platform::SetBrightness(float Brightness)
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Jni::SetBrightness(Brightness);
#else
	UE_LOG(LogXRDeviceKit, Verbose, TEXT("SetBrightness 在非安卓平台是空实现，入参 %f"), Brightness);
	return false;
#endif
}

EXRDeviceScreenOrientation XRDeviceKit::Platform::GetScreenOrientation()
{
#if PLATFORM_ANDROID
	switch (FPlatformMisc::GetDeviceOrientation())
	{
	case EDeviceScreenOrientation::Portrait:
		return EXRDeviceScreenOrientation::Portrait;
	case EDeviceScreenOrientation::PortraitUpsideDown:
		return EXRDeviceScreenOrientation::PortraitUpsideDown;
	case EDeviceScreenOrientation::LandscapeLeft:
		return EXRDeviceScreenOrientation::LandscapeLeft;
	case EDeviceScreenOrientation::LandscapeRight:
		return EXRDeviceScreenOrientation::LandscapeRight;
	default:
		return EXRDeviceScreenOrientation::Unknown;
	}
#else
	return EXRDeviceScreenOrientation::Unknown;
#endif
}

bool XRDeviceKit::Platform::SetScreenOrientation(EXRDeviceScreenOrientation Orientation)
{
#if PLATFORM_ANDROID
	EDeviceScreenOrientation EngineOrientation = EDeviceScreenOrientation::Unknown;
	switch (Orientation)
	{
	case EXRDeviceScreenOrientation::Portrait:
		EngineOrientation = EDeviceScreenOrientation::Portrait;
		break;
	case EXRDeviceScreenOrientation::PortraitUpsideDown:
		EngineOrientation = EDeviceScreenOrientation::PortraitUpsideDown;
		break;
	case EXRDeviceScreenOrientation::LandscapeLeft:
		EngineOrientation = EDeviceScreenOrientation::LandscapeLeft;
		break;
	case EXRDeviceScreenOrientation::LandscapeRight:
		EngineOrientation = EDeviceScreenOrientation::LandscapeRight;
		break;
	default:
		return false;
	}
	FPlatformMisc::SetDeviceOrientation(EngineOrientation);
	return true;
#else
	return false;
#endif
}

// ==================== 网络 ====================

EXRDeviceNetworkType XRDeviceKit::Platform::GetNetworkType()
{
#if PLATFORM_ANDROID
	switch (FPlatformMisc::GetNetworkConnectionType())
	{
	case ENetworkConnectionType::None:
		return EXRDeviceNetworkType::None;
	case ENetworkConnectionType::AirplaneMode:
		return EXRDeviceNetworkType::AirplaneMode;
	case ENetworkConnectionType::Cell:
		return EXRDeviceNetworkType::Cellular;
	case ENetworkConnectionType::WiFi:
		return EXRDeviceNetworkType::WiFi;
	case ENetworkConnectionType::WiMAX:
		return EXRDeviceNetworkType::WiMax;
	default:
		return EXRDeviceNetworkType::Unknown;
	}
#else
	return EXRDeviceNetworkType::Unknown;
#endif
}

bool XRDeviceKit::Platform::HasWiFiConnection()
{
#if PLATFORM_ANDROID
	return FPlatformMisc::HasActiveWiFiConnection();
#else
	return false;
#endif
}

// ==================== 音频 ====================

int32 XRDeviceKit::Platform::GetDeviceVolume()
{
	return FPlatformMisc::GetDeviceVolume();
}

bool XRDeviceKit::Platform::IsHeadphonesPluggedIn()
{
#if PLATFORM_ANDROID
	return FPlatformMisc::AreHeadPhonesPluggedIn();
#else
	return false;
#endif
}

// ==================== 应用信息 ====================

FString XRDeviceKit::Platform::GetVersionName()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetVersionName();
#else
	return FString();
#endif
}

int64 XRDeviceKit::Platform::GetVersionCode()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetVersionCode();
#else
	return -1;
#endif
}

bool XRDeviceKit::Platform::IsAppInstalled(const FString& PackageName)
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::IsAppInstalled(PackageName);
#else
	return false;
#endif
}

// ==================== 屏幕尺寸 ====================

int32 XRDeviceKit::Platform::GetScreenWidth()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetScreenWidth();
#else
	return 0;
#endif
}

int32 XRDeviceKit::Platform::GetScreenHeight()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetScreenHeight();
#else
	return 0;
#endif
}

int32 XRDeviceKit::Platform::GetScreenDpi()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetScreenDpi();
#else
	return 0;
#endif
}

float XRDeviceKit::Platform::GetScreenDensity()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetScreenDensity();
#else
	return 1.0f;
#endif
}

float XRDeviceKit::Platform::GetScreenRefreshRate()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetScreenRefreshRate();
#else
	return 0.0f;
#endif
}

// ==================== 存储 ====================

int64 XRDeviceKit::Platform::GetInternalFreeMegabytes()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetInternalFreeMegabytes();
#else
	return -1;
#endif
}

int64 XRDeviceKit::Platform::GetInternalTotalMegabytes()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetInternalTotalMegabytes();
#else
	return -1;
#endif
}

int64 XRDeviceKit::Platform::GetExternalFreeMegabytes()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetExternalFreeMegabytes();
#else
	return -1;
#endif
}

int64 XRDeviceKit::Platform::GetExternalTotalMegabytes()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetExternalTotalMegabytes();
#else
	return -1;
#endif
}

// ==================== 系统 ====================

int64 XRDeviceKit::Platform::GetUptimeSeconds()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetUptimeSeconds();
#else
	return -1;
#endif
}

FString XRDeviceKit::Platform::GetTimeZoneId()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetTimeZoneId();
#else
	return FString();
#endif
}

FString XRDeviceKit::Platform::GetLanguageTag()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetLanguageTag();
#else
	return FString();
#endif
}

// ==================== 设备构建信息 ====================

FString XRDeviceKit::Platform::GetManufacturer()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetManufacturer();
#else
	return FString();
#endif
}

FString XRDeviceKit::Platform::GetModel()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetModel();
#else
	return FString();
#endif
}

FString XRDeviceKit::Platform::GetDeviceName()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetDeviceName();
#else
	return FString();
#endif
}

FString XRDeviceKit::Platform::GetSupportedABIs()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetSupportedABIs();
#else
	return FString();
#endif
}

// ==================== 应用信息（补充） ====================

int64 XRDeviceKit::Platform::GetFirstInstallTimeMs()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetFirstInstallTimeMs();
#else
	return -1;
#endif
}

int64 XRDeviceKit::Platform::GetLastUpdateTimeMs()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetLastUpdateTimeMs();
#else
	return -1;
#endif
}

FString XRDeviceKit::Platform::GetAppLabel()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetAppLabel();
#else
	return FString();
#endif
}

bool XRDeviceKit::Platform::StartApp(const FString& PackageName)
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::StartApp(PackageName);
#else
	return false;
#endif
}

// ==================== 音频（补充） ====================

bool XRDeviceKit::Platform::SetVolume(int32 Percent)
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::SetVolume(Percent);
#else
	return false;
#endif
}

// ==================== 显示（补充） ====================

int32 XRDeviceKit::Platform::GetSystemBrightness()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetSystemBrightness();
#else
	return -1;
#endif
}

bool XRDeviceKit::Platform::IsNightMode()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::IsNightMode();
#else
	return false;
#endif
}

FString XRDeviceKit::Platform::GetPackageName()
{
#if PLATFORM_ANDROID
	return XRDeviceKit::Java::GetPackageName();
#else
	return FString();
#endif
}

void XRDeviceKit::Platform::CopyToClipboard(const FString& Text)
{
	FPlatformApplicationMisc::ClipboardCopy(*Text);
}

FString XRDeviceKit::Platform::PasteFromClipboard()
{
	FString Result;
	FPlatformApplicationMisc::ClipboardPaste(Result);
	return Result;
}

FString XRDeviceKit::Platform::GetProjectVersion()
{
	// 注意不能用 FApp::GetBuildVersion：那个返回的是引擎构建号（++UE5+Release-...），
	// 工程版本在 Project Settings → Project → Version 里，存于 DefaultGame.ini
	FString ProjectVersion;
	GConfig->GetString(TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectVersion"), ProjectVersion, GGameIni);
	return ProjectVersion;
}

void XRDeviceKit::Platform::ShareURL(const FString& URL, const FString& Description)
{
	// 引擎在 Android 上会拉起系统分享面板；其他平台是空实现
	FPlatformMisc::ShareURL(URL, FText::FromString(Description), 0, 0);
}

// ==================== 振动 ====================

void XRDeviceKit::Platform::StartHaptics(int32 Strength)
{
	EMobileHapticsType HapticsType = EMobileHapticsType::ImpactMedium;
	switch (Strength)
	{
	case 0:
		HapticsType = EMobileHapticsType::ImpactLight;
		break;
	case 1:
		HapticsType = EMobileHapticsType::ImpactMedium;
		break;
	case 2:
		HapticsType = EMobileHapticsType::ImpactHeavy;
		break;
	default:
		break;
	}

	// 先声明要播哪种，再触发；这是引擎要求的调用顺序
	FPlatformMisc::PrepareMobileHaptics(HapticsType);
	FPlatformMisc::TriggerMobileHaptics();
}

void XRDeviceKit::Platform::StopHaptics()
{
	FPlatformMisc::ReleaseMobileHaptics();
}
