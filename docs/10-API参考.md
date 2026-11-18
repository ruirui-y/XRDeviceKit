# XRDeviceKit API 参考

> 版本：0.1.0　引擎：UE 5.5　平台：Android（arm64-v8a）/ Win64（编辑器）
> 实测设备：DPVR P1Max（Lexiang Inc.），Android 12

把 Android 系统能力封装成蓝图节点，拖出来就能用。不需要写 C++，也不需要知道底层走的是引擎接口还是 JNI。

---

## 一、快速开始

1. 把 `Plugins/XRDeviceKit` 整个目录拷进你的工程
2. 打开 `uproject` 启用插件（Edit → Plugins → Device → XR Device Kit）
3. 蓝图里右键搜 `XRDeviceKit`，全部节点按能力分在 `XRDeviceKit|Device`、`XRDeviceKit|Display` 等分类下

## 二、返回值约定

**失败不抛异常，返回固定默认值**，调用方不需要 try/catch：

| 返回类型 | 失败值 |
|---|---|
| `int32` / `int64` | `-1`（少数节点是 `0`，见各节点说明） |
| `float` | `-1.0` 或 `0.0`，见说明 |
| `bool` | `false` |
| `FString` | 空串 |

Win64（编辑器）下所有平台相关节点返回默认值，**不会崩**，工程能正常打开。

---

## 三、设备（17 个）

| 节点 | 说明 | 返回 | P1Max 实测 |
|---|---|---|---|
| `GetBatteryLevel` | 电量百分比 | int32，0~100 | 26 |
| `IsCharging` | 是否在充电（含已充满） | bool | true |
| `IsLowPowerMode` | 系统省电模式 | bool | false |
| `GetBatteryTemperature` | 电池温度，摄氏度 | float | 38.6 |
| `GetCPUTemperature` | CPU 温度，摄氏度 | float，拿不到 -1 | 64.9 |
| `GetDeviceId` | 应用作用域的 ANDROID_ID | FString | 3c17ce7eff30f88c |
| `GetOSVersionString` | 系统版本串 | FString | 12 |
| `GetAndroidMajorVersion` | 安卓主版本号 | int32 | 12 |
| `GetCPUBrand` | CPU 型号（**安卓上返回设备标识，等于型号**） | FString | P1Max |
| `GetCoreCount` | 物理核心数 | int32 | 8 |
| `GetDefaultLocale` | 系统默认语言 | FString | zh_CN |
| `GetDiskFreeMegabytes(Path)` | 指定路径分区剩余空间 MB | int64 | 92764（`/sdcard`） |
| `GetDiskTotalMegabytes(Path)` | 指定路径分区总空间 MB | int64 | 98799 |
| `GetManufacturer` | 厂商 | FString | Lexiang Inc. |
| `GetModel` | 型号 | FString | P1Max |
| `GetDeviceName` | 设备代号 | FString | P1Max |
| `GetSupportedABIs` | 支持的 ABI，逗号分隔 | FString | arm64-v8a,armeabi-v7a,armeabi |

## 四、屏幕（12 个）

| 节点 | 说明 | 返回 | P1Max 实测 |
|---|---|---|---|
| `IsScreenOn` | 屏幕是否亮着（不等于有没有戴上） | bool | true |
| `GetBrightness` | 当前亮度 0~1；窗口跟随系统时返回系统亮度 | float | 0.800 |
| `SetBrightness(Brightness)` | 设置**本 App 窗口**亮度，入参会自动夹到 0~1 | bool | — |
| `GetScreenOrientation` | 屏幕方向枚举 | `EXRDeviceScreenOrientation` | 3（横屏左） |
| `SetScreenOrientation(Orientation)` | 设置屏幕方向 | bool | — |
| `GetScreenWidth` | 宽（像素） | int32 | 1200 |
| `GetScreenHeight` | 高（像素） | int32 | 720 |
| `GetScreenDpi` | 密度 dpi | int32 | 200 |
| `GetScreenDensity` | 密度倍率（160dpi = 1.0） | float | 1.25 |
| `GetScreenRefreshRate` | **设备支持的最高刷新率** Hz | float | 90.0 |
| `GetSystemBrightness` | 系统亮度（平台刻度 0~255），**只读** | int32 | 204 |
| `IsNightMode` | 是否深色模式 | bool | false |

> `EXRDeviceScreenOrientation`：`Unknown` / `Portrait` / `PortraitUpsideDown` / `LandscapeLeft` / `LandscapeRight`

## 五、网络（2 个）

| 节点 | 说明 | 返回 | P1Max 实测 |
|---|---|---|---|
| `GetNetworkType` | 当前网络类型枚举 | `EXRDeviceNetworkType` | 4（WiFi） |
| `HasWiFiConnection` | 是否连着 WiFi | bool | true |

> `EXRDeviceNetworkType`：`Unknown` / `None` / `AirplaneMode` / `Cellular` / `WiFi` / `WiMax`

## 六、音频（3 个）

| 节点 | 说明 | 返回 | P1Max 实测 |
|---|---|---|---|
| `GetDeviceVolume` | 媒体音量百分比 0~100 | int32 | 26 |
| `SetVolume(Percent)` | 设置媒体音量，入参 0~100，内部换算成设备档位 | bool | — |
| `IsHeadphonesPluggedIn` | 是否插着耳机 | bool | false |

## 七、存储（4 个）

| 节点 | 说明 | 返回 | P1Max 实测 |
|---|---|---|---|
| `GetInternalFreeMegabytes` | 内部存储剩余 MB | int64 | 92764 |
| `GetInternalTotalMegabytes` | 内部存储总量 MB | int64 | 98799 |
| `GetExternalFreeMegabytes` | 外部存储剩余 MB | int64 | 92764 |
| `GetExternalTotalMegabytes` | 外部存储总量 MB | int64 | 98799 |

> 读的是 App 所在分区（`/data`），**不是整机标称容量**。P1Max 标称 128G，`/data` 实际 96G。内部和外部在同一分区时四个值只有两个独立值。

## 八、应用（8 个）

| 节点 | 说明 | 返回 | P1Max 实测 |
|---|---|---|---|
| `GetPackageName` | 包名 | FString | com.YourCompany.XRDeviceKitDemo |
| `GetVersionName` | 版本名 | FString | 1.0 |
| `GetVersionCode` | 版本号（构建号） | int64 | 1 |
| `GetAppLabel` | 应用显示名 | FString | XRDeviceKitDemo |
| `GetFirstInstallTimeMs` | 首次安装时间（毫秒时间戳） | int64 | 1782296507278 |
| `GetLastUpdateTimeMs` | 最后更新时间 | int64 | 1794995318567 |
| `IsAppInstalled(PackageName)` | 指定包是否安装（**受 Android 11 包可见性限制**） | bool | — |
| `StartApp(PackageName)` | 拉起指定包（**同样受包可见性限制**） | bool | — |

## 九、系统（7 个）

| 节点 | 说明 | 返回 | P1Max 实测 |
|---|---|---|---|
| `GetUptimeSeconds` | 开机时长（秒），含深睡眠 | int64 | — |
| `GetTimeZoneId` | 时区 ID | FString | America/Los_Angeles |
| `GetLanguageTag` | 语言标签 | FString | zh-CN |
| `GetProjectVersion` | 工程版本（Project Settings → Project → Version） | FString | 1.0.0.0 |
| `CopyToClipboard(Text)` | 写系统剪贴板 | — | — |
| `PasteFromClipboard` | 读系统剪贴板 | FString | 往返一致 |
| `ShareURL(URL, Description)` | 调系统分享面板 | — | — |

## 十、振动（2 个）

| 节点 | 说明 | 返回 |
|---|---|---|
| `StartHaptics(Strength)` | 触发振动，0 轻 / 1 中 / 2 重 | — |
| `StopHaptics` | 停止并释放振动资源 | — |

## 十一、性能（2 个）

| 节点 | 说明 | 返回 | P1Max 实测 |
|---|---|---|---|
| `GetRenderFPS` | 引擎平滑帧率（同 `stat fps`） | float | 72.4 |
| `GetFrameTimeMs` | 每帧耗时 = 1000 / 帧率 | float | 13.8 |

> **帧率和刷新率不是一回事**：`GetScreenRefreshRate` = 90（面板能力），`GetRenderFPS` = 72.4（App 实际渲染帧率，跑在 72Hz 的 2D 面板上）。

---

## 十二、使用注意

| 注意点 | 说明 |
|---|---|
| **亮度只能改自己** | `SetBrightness` 只影响本 App 窗口；改整机亮度要 `WRITE_SETTINGS`，本插件不做 |
| **刷新率是能力值** | 返回设备支持的最高值，不是当前实际帧率，实际帧率看 `GetRenderFPS` |
| **存储是分区容量** | 不是标称容量，见第七节 |
| **两个包相关节点受限** | `StartApp` / `IsAppInstalled` 在 Android 11+ 只能看到清单 `<queries>` 里声明过的包，否则一律返回 false |
| **设备 ID 作用域** | `GetDeviceId` 是 Android 8+ 的**应用级** ANDROID_ID，和 `adb shell settings get secure android_id` 本来就不同；同一签名重装不变，换签名会变 |
| **CPU 温度仅供参考** | 读系统热区文件，不同厂商可能拿不到（返回 -1） |
| **全部在游戏线程调** | 节点内部走 JNI，非游戏线程调用需自行 attach，插件未做保护 |

## 十三、怎么验证

插件自带两层测试：

```powershell
# 1. 契约测试（编辑器，10 秒）：57 个节点的默认值与数量
UnrealEditor-Cmd.exe <工程>.uproject -ExecCmds="Automation RunTests XRDeviceKit;Quit" -unattended -nop4 -nullrhi -log

# 2. 真机冒烟：装包后自动跑，日志每行一个值
adb logcat -d | findstr "SMOKE|"
```

冒烟输出格式：`SMOKE|节点名=值`，可直接和头显设置页逐项对照。

## 十四、版本记录

| 版本 | 日期 | 变更 |
|---|---|---|
| 0.1.0 | 2026-11-18 | 首版：57 个节点，P1Max 实测通过；亮度、刷新率、音量刻度、工程版本按真机对照修正 |
