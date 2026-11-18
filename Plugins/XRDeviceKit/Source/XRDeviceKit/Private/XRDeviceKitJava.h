// Java 层桥接：调用 JavaLibs/xrbridge 里的静态方法。
// 这一层只认 com/xrdevicekit/bridge/XRBridge 这一个类；方法名和签名必须和 XRBridge.java 一致，
// 改了一边没改另一边，真机上日志里会直接报"找不到方法"。

#pragma once

#include "CoreMinimal.h"

namespace XRDeviceKit
{
namespace Java
{
// ---------- 应用信息 ----------
FString GetPackageName();                                                           // 包名，失败空串
FString GetVersionName();                                                           // 版本名 1.0，失败空串
int64 GetVersionCode();                                                             // 版本号，失败 -1
bool IsAppInstalled(const FString& PackageName);                                    // 是否装了某包（受 Android 11 包可见性限制）

// ---------- 屏幕 ----------
int32 GetScreenWidth();                                                             // 屏幕宽（像素），失败 0
int32 GetScreenHeight();                                                            // 屏幕高（像素），失败 0
int32 GetScreenDpi();                                                               // 屏幕密度 dpi，失败 0
float GetScreenDensity();                                                           // 密度倍率，失败 1.0
float GetScreenRefreshRate();                                                       // 刷新率 Hz，失败 0

// ---------- 存储（单位 MB） ----------
int64 GetInternalFreeMegabytes();                                                   // 内部存储剩余，失败 -1
int64 GetInternalTotalMegabytes();                                                  // 内部存储总量，失败 -1
int64 GetExternalFreeMegabytes();                                                   // 外部存储剩余，失败 -1
int64 GetExternalTotalMegabytes();                                                  // 外部存储总量，失败 -1

// ---------- 系统 ----------
int64 GetUptimeSeconds();                                                           // 开机时长（秒），失败 -1
FString GetTimeZoneId();                                                            // 时区 ID，如 Asia/Shanghai
FString GetLanguageTag();                                                           // 语言标签，如 zh-CN

// ---------- 设备构建信息 ----------
FString GetManufacturer();                                                          // 厂商，如 DPVR
FString GetModel();                                                                 // 型号，如 DPVR E4
FString GetDeviceName();                                                            // 设备代号（Build.DEVICE）
FString GetSupportedABIs();                                                         // 支持的 ABI，逗号分隔

// ---------- 应用信息（补充） ----------
int64 GetFirstInstallTimeMs();                                                      // 首次安装时间（毫秒），失败 -1
int64 GetLastUpdateTimeMs();                                                        // 最后更新时间（毫秒），失败 -1
FString GetAppLabel();                                                              // 应用显示名
bool StartApp(const FString& PackageName);                                          // 拉起指定包，受 Android 11 包可见性限制

// ---------- 音频（补充） ----------
bool SetVolume(int32 Percent);                                                      // 设置音量百分比 0~100

// ---------- 显示（补充） ----------
int32 GetSystemBrightness();                                                        // 系统亮度（0~255），失败 -1
bool IsNightMode();                                                                 // 是否深色模式
}
}
