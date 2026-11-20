# 大鹏 SDK 摸底：手柄电量到底在哪

> 日期：2026-11-20
> 素材：文轩给的 `QVR300.zip`（231 MB）、P1Max 设备
> 结论先行：**GSXR 这条路在 P1Max 上走不通；手柄电量在他们的系统服务 `IControllerService` 里，要接口定义才能调。**

---

## 1. 包里有什么

`QVR300` 不是空壳，是高通骁龙 VR（SVR/GSXR）的完整 SDK 源码树：

| 内容 | 说明 |
|---|---|
| `gsxr\jni\arm64-v8a\libgsxr_fe.so` | 38 KB，导出 `GSXR_nativeInitSlam` / `GSXR_nativeGetControllerBattery` / `GSXR_nativeSetControllerBatteryEventCallback` 等 |
| `gsxr\SCGSXRApi.h` | C API 声明，`GSXR_nativeGetControllerBattery(int lr)` 就是手柄电量 |
| `svrApi\libs\svrApi-v8a-release.aar` | 含 arm64 `libsvrapi_core.so`（1.7 MB） |
| `LibGSXR\...\arm64-v8a\libsvrapi_core.so` | 同一套 core，导出 `GSXR_SC_*` 系列 |
| `svrUE4\Plugins\Runtime\SnapdragonVR` | UE4 的 HMD + 手柄插件源码 |
| `doc\*.pdf` | 官方集成指南 / 用户指南 |

---

## 2. 试了 GSXR：库能加载，初始化失败

把 `libgsxr_fe.so` 打进 APK，手写 `dlopen` + `dlsym` 调用：

```cpp
void* Handle = dlopen("libgsxr_fe.so", RTLD_NOW);          // 成功
FnInit Init = dlsym(Handle, "GSXR_nativeInitSlam");        // 成功
FGSXRInitParams Params{JavaVm, JNIEnv, Activity};
int32 Result = Init(&Params);                              // 返回 3（失败）
```

设备日志：

```
I/willie_gsxr_log: GSXR_nativeInitSlam start gIsInitialized=0
E/willie_gsxr_log: GSXR_nativeInitSlam Failed for opening libgsxr_core.so
LogXRDeviceKitGSXR: GSXR 初始化返回 3（0 = 成功）
```

**缺 `libgsxr_core.so`。** 整棵 SDK 树里没有这个文件名，只有 `libsvrapi_core.so`（导出的是 `GSXR_SC_*` 那套，不是 fe 要的）。

---

## 3. 设备侧核查：这台机器没有 GSXR

```bash
# 系统库里没有 gsxr 相关的 so
adb shell "ls /system/lib64 /vendor/lib64 | grep -iE 'gsxr|svr|qvr'"
  libsvrapi.so          ← 有，经典 SVR
  libqvrservice.so
  libqvrservice_client.so
  ...

# 没有 GSXR 服务包
adb shell "pm list packages | grep -i gsxr"    # 空
```

而且把设备上的 `/system/lib64/libsvrapi.so` 拉下来扫符号：

```
svrInitialize:                     有
GSXR_SC_controllerGetBattery:      没有
```

**结论：GSXR 前端库在这台设备上没有对应的 core 和服务；系统的 SVR 库也不带 GSXR_SC 那套电池接口。**

---

## 4. SVR 公开 API 里也没有电池

`svrApi\public\svrApi.h`（版本较全的头文件）里：

```c
enum svrControllerQueryType {
    kControllerQueryBatteryRemaining = 0,   // 只能"发查询消息"
    kControllerQueryControllerCaps = 1
};
```

但 `svrControllerState` 结构体里**只有连接状态，没有电量字段**，`svrEventType` 里也没有电池事件。也就是说经典 SVR 只给你"问"的口子，没给你"收答案"的回调（那是 `GSXR_SC_controllerSetBatteryEventCallback` 干的，而设备上没有）。

---

## 5. 真正的位置：大鹏自己的系统服务

```bash
adb shell "dumpsys package com.dpvr.controller | grep -i service"
  com.dpvr.controller/.device.ControllerDeviceService
  codePath=/system/priv-app/HtcControllerService
```

把它拉下来（4.6 MB）扫 dex 字符串，里面有：

```
Lcom/dpvr/controller/manager/IControllerService          ← AIDL 接口
Lcom/dpvr/controller/manager/IControllerServiceListener
getBatteryStatus / hasBatteryStatus / setBatteryStatus
onBatteryLevel / mbatteryLevel / BatteryPercentage / DeviceBatteryPercentage
INVALID: Attr_DeviceBatteryPercentage_Float 等常量
```

**手柄电量就在 `IControllerService.getBatteryStatus()` 里**，走的是系统 AIDL 服务，不是任何 SDK 的 C API。

---

## 6. 卡在哪

| 缺口 | 影响 |
|---|---|
| 没有 AIDL 接口定义（`.aidl` 或编译好的 jar） | 只能逆向 transaction code，工作量大且脆弱 |
| 不知道绑定服务的权限要求 | 直接 bind 可能被 `SecurityException` 拒绝 |
| 测试时手柄没连上（`bluetooth_manager` 显示 `STATE_DISCONNECTED`，配对列表为空） | 即使接口通了，也读不到值 |

---

## 7. 下一步

**首选：找文轩要三样东西**

1. `IControllerService` 的 AIDL 文件或 jar
2. 绑定 `com.dpvr.controller` 需要什么权限，我们的 App 能不能拿到
3. 一段他们自己的调用示例（Java 就行）

**备选：自己逆向**

从 `HtcControllerService.apk` 里解出 `IControllerService` 的 transaction 顺序，写一个 Java 客户端 bind 服务。可行，但没有权限的话照样失败，最后还是要问他们。

**顺带：** 让手柄开机并连上头显再测一次，否则任何接口都读不到电量。

---

## 8. 复现命令

```powershell
# 看设备有没有 GSXR/SVR
adb shell "ls /system/lib64 | grep -iE 'gsxr|svr'"

# 拉系统 SVR 库并扫符号
adb pull /system/lib64/libsvrapi.so
# 扫 GSXR_SC_controllerGetBattery / svrInitialize

# 找控制器服务
adb shell "dumpsys package com.dpvr.controller | grep -i service"
adb pull /system/priv-app/HtcControllerService/HtcControllerService.apk
# 扫 dex 里的 IControllerService / getBatteryStatus
```

> `QVR300/` 和拉下来的 `_dev_*` 文件都是外部素材，已加进 `.gitignore`，不进版本库。
