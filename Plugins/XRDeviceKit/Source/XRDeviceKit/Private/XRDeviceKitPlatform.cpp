// 平台层的唯一实现文件。
// 这里是全插件唯一出现 #if PLATFORM_ANDROID 的地方：上面是真实现，下面是给编辑器
// （Win64 / Mac）用的假数据，保证没有安卓设备时工程也能正常打开和调试。

#include "XRDeviceKitPlatform.h"
#include "XRDeviceKitJni.h"

DEFINE_LOG_CATEGORY_STATIC(LogXRDeviceKit, Log, All);

int32 XRDeviceKit::Platform::GetBatteryLevel()
{
	// 引擎自带接口：Android 读系统广播拿到 0~100，其他平台内部直接返回 -1，不用分支
	return FPlatformMisc::GetBatteryLevel();
}

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
