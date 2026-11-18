// 平台层的唯一实现文件。
// 这里是全插件唯一出现 #if PLATFORM_ANDROID 的地方：上面是真实现，下面是给编辑器
// （Win64 / Mac）用的假数据，保证没有安卓设备时工程也能正常打开和调试。

#include "XRDeviceKitPlatform.h"
#include "XRDeviceKitJni.h"
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

// ==================== 系统 ====================

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
	return FString(FApp::GetBuildVersion());
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
