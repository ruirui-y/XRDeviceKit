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
// ---------- 零权限查询（补充） ----------
FString GetIpAddress();                                                             // 本机 IPv4，失败空串
int32 GetScreenOffTimeout();                                                        // 息屏超时（毫秒），失败 -1
FString GetLanguageList();                                                          // 语言列表，逗号分隔
int32 GetRingerMode();                                                              // 0 静音 / 1 振动 / 2 正常，-1 未知
int32 GetBatteryCurrentNow();                                                       // 电池电流（毫安），符号看厂商，-1 不支持
int32 GetBatteryHealth();                                                           // 电池健康，-1 未知

// ---------- 事件监听开关 ----------
void StartScreenWatcher();                                                          // 开始监听亮屏/灭屏
void StopScreenWatcher();                                                           // 停止监听亮屏/灭屏
void StartNetworkWatcher();                                                         // 开始监听网络变化
void StopNetworkWatcher();                                                          // 停止监听网络变化

// ---------- 大鹏 SDK 服务探针 ----------
void StartSdkServiceProbe();                                                        // bind com.deepoon.sdk.manager 并打印描述符
}
}
