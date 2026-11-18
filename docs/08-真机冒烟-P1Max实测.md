# 真机冒烟：P1Max 上跑完 48 个节点

> 日期：2026-11-18
> 设备：DPVR P1Max（`Lexiang Inc.`），Android 12，arm64
> 前置：[07-测试-契约与冒烟.md](07-测试-契约与冒烟.md)

---

## 1. 这一阶段要做什么

56 个节点写完了，但一条真机数据都没有。这轮把包装到头显上，让冒烟测试自己跑，看哪些值是对的、哪些是错的。

**做法**：`SmokeTestOnStartup` 默认开 → 装包 → `adb logcat` 抓 `SMOKE|` 前缀。

---

## 2. 怎么做的

```powershell
adb install -r XRDeviceKitDemo-arm64.apk
adb push main.1.com.YourCompany.XRDeviceKitDemo.obb /sdcard/Android/obb/com.YourCompany.XRDeviceKitDemo/
adb shell am start -n com.YourCompany.XRDeviceKitDemo/com.epicgames.unreal.GameActivity
adb logcat -d | findstr "SMOKE|"
```

引擎启动 1.41 秒，冒烟在 `Initialize` 后 3 秒触发，**48 个值一次全部取到**，没有崩溃。

---

## 3. 写完为什么不是理想效果

三个值明显不对：

### 3.1 `GetProjectVersion` 返回的是引擎版本

```
GetProjectVersion=++UE5+Release-5.5-CL-40574608
```

节点注释写的是"工程版本号（Project Settings → Project → Version）"，实际拿到的是**引擎的构建号**。

原因是实现用了 `FApp::GetBuildVersion()`——那个函数返回的从来就是引擎版本，跟工程版本没关系。

### 3.2 音量两个数对不上

```
GetDeviceVolume=26
GetMaxVolume=15
```

音量 26 > 最大 15，明显不是一个刻度。

查引擎源码 `AndroidPlatformMisc.cpp`：

```cpp
int32 FAndroidMisc::GetDeviceVolume()
{
	//FAndroidMisc::GetVolumeState returns 0-15, scale to 0-100
	int32 BaseVolume = FAndroidMisc::GetVolumeState();
	int32 ScaledVolume = (BaseVolume * 100) / 15;
	return ScaledVolume;
}
```

**引擎返回的是 0~100 百分比**，而 `GetMaxVolume`（走 Java）返回的是设备自己的档位数 15。两个节点单位不同，放一起用必然出错。

### 3.3 `GetCPUBrand` 返回的是设备型号

```
GetCPUBrand=P1Max
GetModel=P1Max
```

两个一样。安卓上引擎这个接口底层读的不是 CPU 型号。

---

## 4. 怎么定位的

不看代码猜，直接看值：

| 值 | 判断 |
|---|---|
| `GetProjectVersion` 带 `++UE5+Release` 前缀 | 这是引擎构建号的特征串，不可能是工程版本 |
| 26 > 15 | 两个数不在同一量纲，必有其一用错 |
| `GetCPUBrand` == `GetModel` | 撞车了，说明取的是同一个字段 |

对 3.2 直接去引擎源码确认了换算公式（0-15 → 0-100），不是猜的。

---

## 5. 怎么解决的

| 问题 | 改法 |
|---|---|
| 工程版本取错 | `FApp::GetBuildVersion()` → `GConfig->GetString(... GeneralProjectSettings / ProjectVersion ...)` |
| 音量刻度不一 | 统一到 0~100：`SetVolume` 改收百分比（Java 内部换算成档位），**删掉 `GetMaxVolume`**（百分比体系下没意义） |
| CPU 品牌误导 | 不改实现（那是引擎行为），注释里写明"安卓上返回设备标识，实测等于型号" |

删节点后总数 56 → 55，契约测试的数量断言同步改成 `>= 55`。

---

## 6. 最终效果

### 第二轮冒烟（修复后重打包）

```
条数: 49
GetDeviceVolume=26          ← 百分比，26%
GetProjectVersion=1.0.0.0   ← 不再是引擎构建号
（GetMaxVolume 已删除）
```

### 48 个值的实测结果

| 分组 | 结果 |
|---|---|
| 电池 | 电量 29%、充电中、电池 36.4°C、CPU 56.9°C |
| 设备 | 厂商 `Lexiang Inc.`、型号 `P1Max`、核心 8、ID `3c17ce7eff30f88c` |
| 系统 | Android 12、语言 `zh_CN` / `zh-CN`、时区 `America/Los_Angeles` |
| 屏幕 | 1200×720、DPI 200、密度 1.25、刷新率 72Hz、方向 3（横屏左） |
| 存储 | 内部 92764/98799 MB，外部同值（同一分区） |
| 音量 | 26%（0~100） |
| 应用 | 包名、版本 1.0 / 1、显示名 `XRDeviceKitDemo` |
| 剪贴板 | 写入 `XRDeviceKitSmoke` 后读回一致 |
| 网络 | 类型 1（None）、无 WiFi——设备确实没连网（`dumpsys connectivity` 显示 `Active default network: none`） |

### 本轮结论

- **JNI 链路全通**：28 个 Java 能力全部取到真值，没有一个是空/默认值
- **注入链路全通**：`XRDeviceKit_IsScreenOn` 等 3 个 GameActivity 方法正常返回
- **3 个真实问题**：1 个是纯 bug（工程版本），1 个是 API 设计问题（音量刻度），1 个是注释误导

### 还没解决的

1. `GetProjectVersion` 读到的是 `1.0.0.0`——默认值。我在 `DefaultGame.ini` 里写的 `ProjectVersion=0.1.0` 没进烤包，原因待查（可能是 ProjectSettings 加载时把版本号规范化了）。工程版本建议直接在编辑器 Project Settings 里设。
2. `GetTimeZoneId=America/Los_Angeles` 但语言是 `zh-CN`——设备时区设置，不是代码问题，需要确认是不是你们要的。
