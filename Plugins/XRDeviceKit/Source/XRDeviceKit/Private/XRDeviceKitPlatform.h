// 平台层：蓝图库和具体实现之间的唯一接口。
// Android 实现在 XRDeviceKitPlatform.cpp 的 #if PLATFORM_ANDROID 分支里，
// 其他平台在同一文件的 #else 分支里返回默认值。

#pragma once

#include "CoreMinimal.h"
#include "XRDeviceKitTypes.h"

namespace XRDeviceKit
{
namespace Platform
{
// ---------- 设备 / 电源 ----------
int32 GetBatteryLevel();                                                            // 0~100，失败 -1
bool IsCharging();                                                                  // 正在充电或已充满
bool IsLowPowerMode();                                                              // 系统省电模式
float GetBatteryTemperature();                                                      // 电池温度，摄氏度；失败 -1
float GetCPUTemperature();                                                          // CPU 温度，摄氏度；失败 -1
FString GetDeviceId();                                                              // 设备唯一 ID
FString GetOSVersionString();                                                       // 系统版本号字符串
int32 GetAndroidMajorVersion();                                                     // 安卓主版本号，如 12；非安卓 -1
FString GetCPUBrand();                                                              // CPU 型号
int32 GetCoreCount();                                                               // 物理核心数
FString GetDefaultLocale();                                                         // 系统默认语言，如 zh-Hans-CN
int64 GetDiskFreeMegabytes(const FString& Path);                                    // 剩余空间 MB，失败 -1
int64 GetDiskTotalMegabytes(const FString& Path);                                   // 总空间 MB，失败 -1

// ---------- 显示 ----------
bool IsScreenOn();                                                                  // 屏幕是否亮着
float GetBrightness();                                                              // 本窗口亮度 0~1，-1 跟随系统
bool SetBrightness(float Brightness);                                               // 设置本窗口亮度
EXRDeviceScreenOrientation GetScreenOrientation();                                  // 当前屏幕方向
bool SetScreenOrientation(EXRDeviceScreenOrientation Orientation);                  // 设置屏幕方向

// ---------- 网络 ----------
EXRDeviceNetworkType GetNetworkType();                                              // 当前网络类型
bool HasWiFiConnection();                                                           // 是否连着 WiFi

// ---------- 音频 ----------
int32 GetDeviceVolume();                                                            // 音量档位 0~15
bool IsHeadphonesPluggedIn();                                                       // 是否插着耳机

// ---------- 系统 ----------
FString GetPackageName();                                                           // 运行中的包名
void CopyToClipboard(const FString& Text);                                          // 写剪贴板
FString PasteFromClipboard();                                                       // 读剪贴板
FString GetProjectVersion();                                                        // 工程版本号
void ShareURL(const FString& URL, const FString& Description);                      // 调系统分享

// ---------- 振动 ----------
void StartHaptics(int32 Strength);                                                  // 0 轻 / 1 中 / 2 重
void StopHaptics();                                                                 // 停止并释放振动资源
}
}
