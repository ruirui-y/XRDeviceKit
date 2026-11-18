// 平台层：蓝图库和具体实现之间的唯一接口。
// Android 实现在 XRDeviceKitPlatform.cpp 的 #if PLATFORM_ANDROID 分支里，
// 其他平台在同一个文件的 #else 分支里返回默认值。

#pragma once

#include "CoreMinimal.h"

namespace XRDeviceKit
{
namespace Platform
{
// 电量百分比 0~100，拿不到返回 -1
int32 GetBatteryLevel();

// 屏幕是否亮着；非 Android 一律当亮着
bool IsScreenOn();

// 本窗口亮度 0~1，-1 表示跟随系统
float GetBrightness();

// 设置本窗口亮度，入参已被上层夹到 0~1
bool SetBrightness(float Brightness);
}
}
