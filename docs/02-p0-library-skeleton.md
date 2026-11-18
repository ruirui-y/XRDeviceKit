# P0：插件骨架与三个设备 API

> 日期：2026-11-18
> 代码提交：见 git log `feat(plugin): P0 ...`
> 前置：[01-plan.md](01-plan.md)

---

## 1. 这一阶段要做什么

**目标**：把「蓝图 → 平台层 → 引擎 / JNI → Android Java」整条链路跑通，用三个 API 当样本，把后面 100 多个 API 的模板定下来。

**范围**

| 做 | 不做 |
|---|---|
| 电量（引擎自带，验证 UFUNCTION 链路） | 大鹏 SDK（还没到） |
| 屏幕亮灭（走 JNI，验证 Java 注入） | 事件订阅（Subsystem 阶段再做） |
| 窗口亮度读写（走 JNI，带参数往返） | 需要系统权限的能力（二档） |
| Win64 编译验证 | 安卓真机验证（没有设备） |

---

## 2. 怎么做的

### 2.1 分层

```
蓝图 / UMG
   │
   ▼
UXRDeviceKitLibrary             Public/XRDeviceKitLibrary.h        只转发
   │
   ▼
XRDeviceKit::Platform           Private/XRDeviceKitPlatform.cpp    唯一有 #if 的地方
   │
   ├───────────────────────┐
   ▼                       ▼
FPlatformMisc::GetBatteryLevel()   XRDeviceKit::Jni                  Private/XRDeviceKitJni.cpp
                           │        缓存 jmethodID，拼签名
                           ▼
                     注入 GameActivity 的 Java                    Android/XRDeviceKit_APL.xml
                           │
                           ▼
                     Android 框架（PowerManager / WindowManager）
```

### 2.2 四个关键决策

| 决策 | 选的 | 没选的 | 理由 |
|---|---|---|---|
| 平台分支放哪 | 集中在 `Platform.cpp`，每个函数一个 `#if` | 蓝图库里逐个 `#if` | 130 个 API 时，蓝图库要读起来像目录，不能全是宏 |
| 蓝图库形态 | `UBlueprintFunctionLibrary` + 静态函数 | Subsystem 实例函数 | 蓝图里不用 `GetGameInstance`，拖出来就能用 |
| 亮度改谁 | 本 App 窗口亮度 | 系统亮度 | 系统亮度要 `WRITE_SETTINGS`，普通 App 拿不到；窗口亮度零权限 |
| 亮度怎么过 JNI | C++ 换算成 0~100 的 `int` 传 | 直接传 `float` | JNI 的可变参数接口会把 `float` 默认提升成 `double`，部分 ABI 上读错寄存器 |

### 2.3 文件清单

| 文件 | 行数 | 职责 |
|---|---|---|
| `Public/XRDeviceKitLibrary.h` | 36 | 4 个 `UFUNCTION` 声明（SetBrightness 是第 4 个） |
| `Private/XRDeviceKitLibrary.cpp` | 30 | 转发 + 参数夹紧 |
| `Private/XRDeviceKitPlatform.h` | 26 | `XRDeviceKit::Platform` 接口 |
| `Private/XRDeviceKitPlatform.cpp` | 45 | Android 真实现 + 其他平台假数据 |
| `Private/XRDeviceKitJni.h` | 22 | JNI 层接口 |
| `Private/XRDeviceKitJni.cpp` | 85 | 方法 ID 缓存 + 3 个调用 |
| `Android/XRDeviceKit_APL.xml` | 76 | 注入 3 个 Java 方法 |
| `XRDeviceKit.Build.cs` | 31 | Android 分支登记 UPL、补模块 |

---

## 3. 写完为什么不是理想效果

代码一次编译过，但**验证过程本身踩了三个坑**，其中两个会直接坑到真机。

### 坑 1：验证脚本把 XML 读成乱码，报了个假错

用 PowerShell 校验 XML，报：

```
"<"(十六进制值 0x3C)是无效的特性字符。 第 8 行，位置 2。
```

同时控制台打出来的文件内容是这样的：

```
XRDeviceKit 鐨?Android 鎵撳寘琛ヤ竵锛圲PL锛夈€?
```

### 坑 2（真隐患）：注入的 Java 里写了中文注释

这份 XML 的 `<insert>` 段会被原样塞进 `GameActivity.java`，再交给 javac。javac 的默认源码编码跟随平台——中文 Windows 上是 GBK。UTF-8 的中文注释被按 GBK 解，轻则乱码，重则报非法字符。

### 坑 3（真隐患）：XML 注释里写了 `$${...}$$` 字面量

UPL 是靠 `$${tag}$$` 这种标记做替换的。注释里出现同样的字面量，属于"赌它不会误伤"，没必要赌。

---

## 4. 怎么定位的

1. 先怀疑文件本身坏了 → 用 `[System.IO.File]::ReadAllText(path, UTF8)` 重新读，解析通过，**排除文件问题**。
2. 再看 PowerShell 行为：`Get-Content` 不带 `-Encoding` 时按 ANSI（中文系统是 GBK）解码 → **假错的根因是验证脚本**，不是文件。
3. 顺着编码这条线往下想：真正会读到这份 Chinese UTF-8 的，是 **javac**，不是 PowerShell。于是查 javac 默认编码 → 跟随平台 → **中文注释有风险**。
4. 再看 UPL 的替换机制，确认 XML 注释里不该出现标记字面量。

---

## 5. 怎么解决的

| 问题 | 改法 |
|---|---|
| 验证脚本假错 | 校验一律用 `[System.IO.File]::ReadAllText($p, UTF8)`，不用 `Get-Content` |
| javac 编码风险 | `<insert>` 里的 **Java 注释全部改英文**；中文只留在 XML 注释里（不会进 Java 文件） |
| 标记字面量 | 注释改成"gameActivityClassAdditions 占位符"，不写 `$` 符号 |

改完的校验结果：

```
XML OK  root=root
initLog=XRDeviceKit: inject GameActivity additions (screen state / window brightness)
Java 方法数: 3
BOM: 60,63,120（无 BOM，`<?x` 开头）  文件字节: 2666
```

---

## 6. 最终效果

### 编译结果（本机实测）

```
[1/7] Compile [x64] XRDeviceKitJni.cpp
[2/7] Compile [x64] XRDeviceKitPlatform.cpp
[3/7] Compile [x64] XRDeviceKitLibrary.cpp
[4/7] Module.XRDeviceKit.cpp
[5/7] Link [x64] UnrealEditor-XRDeviceKit.lib
[6/7] Link [x64] UnrealEditor-XRDeviceKit.dll
[7/7] WriteMetadata XRDeviceKitDemoEditor.target
Total execution time: 10.76 seconds
[status: completed, exit code: 0]
```

### 验收状态

| 项 | 状态 |
|---|---|
| Win64 编译通过 | ✅ |
| XML 格式合法、无 BOM、3 个 Java 方法 | ✅ |
| UPL 被 UAT 收集（`Intermediate/Android/ActiveUPL.txt` 里能看到） | ⬜ 待安卓打包 |
| 安卓包编译通过 | ⬜ 待安卓打包 |
| 真机三个节点返回正确值 | ⬜ 待真机 |

**没验证的部分就是没验证，不能因为 Win64 过了就说"通了"。** JNI 相关代码在 Win64 下整段被 `#if PLATFORM_ANDROID` 剔掉，一行都没执行过。
