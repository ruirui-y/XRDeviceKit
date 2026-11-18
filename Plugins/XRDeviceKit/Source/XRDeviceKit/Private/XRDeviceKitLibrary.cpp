// 蓝图函数的实现：只做参数处理，然后把活交给平台层。

#include "XRDeviceKitLibrary.h"
#include "XRDeviceKitPlatform.h"

int32 UXRDeviceKitLibrary::GetBatteryLevel()
{
	return XRDeviceKit::Platform::GetBatteryLevel();
}

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
