// JNI 层：把注入到 GameActivity 的 Java 方法包成普通 C++ 函数。
// 方法 ID 在这层查一次就缓存，上层不用关心 JNIEnv 和签名。

#pragma once

#include "CoreMinimal.h"

namespace XRDeviceKit
{
namespace Jni
{
// 屏幕是否亮着；拿不到 JNIEnv 时返回 false
bool IsScreenOn();

// 本窗口亮度 0~1，-1 表示跟随系统
float GetBrightness();

// 设置本窗口亮度，入参 0~1
bool SetBrightness(float Brightness);
}
}
