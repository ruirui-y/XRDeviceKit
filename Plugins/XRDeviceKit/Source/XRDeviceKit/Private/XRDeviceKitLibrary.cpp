// 蓝图函数的实现：只做参数处理和区间夹紧，然后把活交给平台层。

#include "XRDeviceKitLibrary.h"
#include "XRDeviceKitPlatform.h"

// ==================== 设备 / 电源 ====================

int32 UXRDeviceKitLibrary::GetBatteryLevel()
{
	return XRDeviceKit::Platform::GetBatteryLevel();
}

bool UXRDeviceKitLibrary::IsCharging()
{
	return XRDeviceKit::Platform::IsCharging();
}

bool UXRDeviceKitLibrary::IsLowPowerMode()
{
	return XRDeviceKit::Platform::IsLowPowerMode();
}

float UXRDeviceKitLibrary::GetBatteryTemperature()
{
	return XRDeviceKit::Platform::GetBatteryTemperature();
}

float UXRDeviceKitLibrary::GetCPUTemperature()
{
	return XRDeviceKit::Platform::GetCPUTemperature();
}

FString UXRDeviceKitLibrary::GetDeviceId()
{
	return XRDeviceKit::Platform::GetDeviceId();
}

FString UXRDeviceKitLibrary::GetOSVersionString()
{
	return XRDeviceKit::Platform::GetOSVersionString();
}

int32 UXRDeviceKitLibrary::GetAndroidMajorVersion()
{
	return XRDeviceKit::Platform::GetAndroidMajorVersion();
}

FString UXRDeviceKitLibrary::GetCPUBrand()
{
	return XRDeviceKit::Platform::GetCPUBrand();
}

int32 UXRDeviceKitLibrary::GetCoreCount()
{
	return XRDeviceKit::Platform::GetCoreCount();
}

FString UXRDeviceKitLibrary::GetDefaultLocale()
{
	return XRDeviceKit::Platform::GetDefaultLocale();
}

int64 UXRDeviceKitLibrary::GetDiskFreeMegabytes(const FString& Path)
{
	return XRDeviceKit::Platform::GetDiskFreeMegabytes(Path);
}

int64 UXRDeviceKitLibrary::GetDiskTotalMegabytes(const FString& Path)
{
	return XRDeviceKit::Platform::GetDiskTotalMegabytes(Path);
}

// ==================== 显示 ====================

bool UXRDeviceKitLibrary::IsScreenOn()
{
	return XRDeviceKit::Platform::IsScreenOn();
}

float UXRDeviceKitLibrary::GetBrightness()
{
	return XRDeviceKit::Platform::GetBrightness();
}

bool UXRDeviceKitLibrary::SetBrightness(float Brightness)
{
	// 蓝图传进来的值不受约束，先夹到合法区间再往下走
	const float Clamped = FMath::Clamp(Brightness, 0.0f, 1.0f);
	return XRDeviceKit::Platform::SetBrightness(Clamped);
}

EXRDeviceScreenOrientation UXRDeviceKitLibrary::GetScreenOrientation()
{
	return XRDeviceKit::Platform::GetScreenOrientation();
}

bool UXRDeviceKitLibrary::SetScreenOrientation(EXRDeviceScreenOrientation Orientation)
{
	return XRDeviceKit::Platform::SetScreenOrientation(Orientation);
}

// ==================== 网络 ====================

EXRDeviceNetworkType UXRDeviceKitLibrary::GetNetworkType()
{
	return XRDeviceKit::Platform::GetNetworkType();
}

bool UXRDeviceKitLibrary::HasWiFiConnection()
{
	return XRDeviceKit::Platform::HasWiFiConnection();
}

// ==================== 音频 ====================

int32 UXRDeviceKitLibrary::GetDeviceVolume()
{
	return XRDeviceKit::Platform::GetDeviceVolume();
}

bool UXRDeviceKitLibrary::IsHeadphonesPluggedIn()
{
	return XRDeviceKit::Platform::IsHeadphonesPluggedIn();
}

// ==================== 系统 ====================

FString UXRDeviceKitLibrary::GetPackageName()
{
	return XRDeviceKit::Platform::GetPackageName();
}

void UXRDeviceKitLibrary::CopyToClipboard(const FString& Text)
{
	XRDeviceKit::Platform::CopyToClipboard(Text);
}

FString UXRDeviceKitLibrary::PasteFromClipboard()
{
	return XRDeviceKit::Platform::PasteFromClipboard();
}

FString UXRDeviceKitLibrary::GetProjectVersion()
{
	return XRDeviceKit::Platform::GetProjectVersion();
}

void UXRDeviceKitLibrary::ShareURL(const FString& URL, const FString& Description)
{
	XRDeviceKit::Platform::ShareURL(URL, Description);
}

// ==================== 振动 ====================

void UXRDeviceKitLibrary::StartHaptics(int32 Strength)
{
	// 只认 0/1/2 三档，其他值统一按中等处理
	const int32 Clamped = FMath::Clamp(Strength, 0, 2);
	XRDeviceKit::Platform::StartHaptics(Clamped);
}

void UXRDeviceKitLibrary::StopHaptics()
{
	XRDeviceKit::Platform::StopHaptics();
}
