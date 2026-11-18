# UE 安卓层与 JNI：我们这次讨论的全部结论

> 记录时间：2026-11-18
> 参考工程：`C:\Users\Mars\UE5Project\DemandClient`（PICO 播控客户端，**只当写法参考，不进本工程**）
> 本工程：`XRDeviceKitDemo` / 插件 `XRDeviceKit`

---

## 0. 这篇解决什么

做插件之前卡住的几个问题：

1. UE 接的到底是不是"它自己的安卓系统"？
2. XML 到底干了什么？写了之后怎么生效？
3. JNI 是什么，能干什么、不能干什么？
4. `.so` 是不是就是一个 lib？和 `aar` 有什么区别？
5. 大鹏给了 SDK 之后，代码该写在哪？

下面每一条都附了参考工程的证据（文件:行号），不是推测。

---

## 1. UE 在安卓上是什么

**结论：UE 没有自己的安卓系统。** 打包出来的就是一个普通 Android App：

```
APK
├─ classes.dex          ← GameActivity.java（UE 的安卓壳子，约 7900 行 Java）
└─ lib/arm64-v8a/
   └─ libUnreal.so      ← 你的 C++ 全编在这里
```

`GameActivity` 负责建 SurfaceView、加载 `libUnreal.so`、把 Activity 对象交给 native。之后你写的所有 C++ 都跑在这个 `.so` 里。

所以"用 UE 的安卓能力"= **UE 已经替你包好的那几个 C++ 接口**（`FPlatformMisc` 里的一部分），其余一律得自己搭桥。

---

## 2. XML 是信封，不是代码

### 2.1 它只装三张纸条

| 纸条 | 内容 | 打包时被放到 |
|---|---|---|
| ① Java 源码 | 真正干活的代码 | 被塞进 `GameActivity.java` |
| ② Manifest 片段 | `uses-permission`、`queries` | 被塞进 `AndroidManifest.xml` |
| ③ Gradle 片段 | jar/aar 依赖、maven 仓库 | 被塞进 `build.gradle` |

**XML 本身不产生任何功能。** 它不会调用 Android 接口，它只是"把别人写好的 Java 搬进 APK"的施工单。

### 2.2 生效链路（4 步）

```
① Build.cs 登记
   AdditionalPropertiesForReceipt.Add("AndroidPlugin", ..._APL.xml)
        │
        ▼
② UAT 收集：所有登记过的 XML 汇总成
   Intermediate/Android/ActiveUPL.txt
        │
        ▼
③ 替换模板占位符
   Engine/Build/Android/Java/src/com/epicgames/unreal/GameActivity.java.template
   第 613 行： //$${gameActivityClassAdditions}$$
        │
        ▼
④ 生成 → Intermediate/Android/arm64/src/com/epicgames/unreal/GameActivity.java
   再由 gradle 编成 classes.dex
```

### 2.3 参考工程里的实测证据

| 证据 | 位置 |
|---|---|
| 登记那一行 | `DemandClient/Source/DemandClient/DemandClient.Build.cs:26` |
| 收集结果（11 条，含我们自己的和 PICO 的） | `Intermediate/Android/ActiveUPL.txt` |
| 模板占位符 | `GameActivity.java.template:613` |
| 注入的 Java 真的在生成文件里 | 生成的 `GameActivity.java:810`（`public int GetDeviceBatteryLevel()`） |

**结论：改完 XML，编辑器里点 Play 不会重新生成，必须重新 Package。**

---

## 3. JNI：C++ 和 Java 之间只有这一根线

### 3.1 四个阶段

```
写代码期   你写 C++ 调用、写 Java 实现
打包期     XML → GameActivity.java（见第 2 节）
启动期     JNI_OnLoad 一次性把 GameActivity 的 jclass 缓存成全局引用
运行期     GetMethodID(名字, 签名) → CallXxxMethod → 执行 Java
```

启动期那一步在 `AndroidJNI.cpp:1911`（`JNI_OnLoad`）→ `:74-86`（`FindClassesAndMethods`，把 `GameActivityClassID` 存成 global ref）。

### 3.2 两种调用写法

```cpp
// 写法 A：调 GameActivity 上注入的方法（PICO 全用这个）
JNIEnv* Env = FAndroidApplication::GetJavaEnv();
static jmethodID M = FJavaWrapper::FindMethod(
    Env, FJavaWrapper::GameActivityClassID, "GetControllerBattery", "(I)[I", false);
FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, M, 0);

// 写法 B：调独立 Java 类
jclass Cls = FAndroidApplication::FindJavaClass("com/yourco/androidbridge/AndroidBridge");
jmethodID M = Env->GetStaticMethodID(Cls, "GetBattery", "()I");
Env->CallStaticIntMethod(Cls, M);
```

`FindJavaClass` 不是裸的 `Env->FindClass`，它走**缓存下来的 ClassLoader**（`AndroidJavaEnv.cpp:187`），所以在游戏线程也能找到应用自己的类。

### 3.3 签名写错 = 运行时死

| Java | 签名 |
|---|---|
| `void` / `int` / `boolean` / `float` | `V` / `I` / `Z` / `F` |
| `String` | `Ljava/lang/String;` |
| `int[]` / `String[]` | `[I` / `[Ljava/lang/String;` |
| 无参返回 int | `()I` |
| 两个 int 参数、返回 void | `(II)V` |

`bIsOptional` 传 `false` 时，名字或签名错了**开发包当场断言崩溃**（`AndroidJNI.cpp:60-70`）。Shipping 包更糟：异常被清掉，`jmethodID` 是 0，继续调用属于未定义行为。

### 3.4 反向：Java 回调 C++

Java 里声明 native 方法，C++ 里实现，名字是硬规则：

```
Java_<包名点换下划线>_<类名>_<方法名>
Java_com_epicgames_unreal_GameActivity_JavaToCBindTobService
```

参考：`PICO_EnterpriseFunctionLibrary.cpp:58`。

**只在厂商 SDK 是异步的时候才需要**（绑服务、投屏状态、升级进度）。

---

## 4. `.so` 不是 lib，是动态库

|  | Windows | Android | 编译期 | 运行期 |
|---|---|---|---|---|
| 静态库 | `.lib`（静态） | `.a` | 整个拷进产物 | 不需要 |
| 动态库 | `.dll` | **`.so`** | 只记符号 | **必须存在，否则起不来** |

用起来像 lib（有头文件、直接调），但差一个致命点：**编译过了不代表能跑**。

### 4.1 两个必须做的动作

```
1. 打进 APK        lib/arm64-v8a/libdpvr.so      ← XML 的 copyFile 或 Build.cs
2. 运行时能找到     System.loadLibrary / dlopen
3. 依赖补齐        它自己依赖的 .so（如 libc++_shared.so）也要打进去
```

第 3 条最容易翻车：只打一个 `.so`，`dlopen` 报 `library "xxx" not found`。

### 4.2 两种接法

```
方式 A：链接期（像用 lib）
   Build.cs: PublicAdditionalLibraries + PublicIncludePaths + RuntimeDependencies
   优点：调用像普通函数，编译期能查签名
   缺点：.so 缺失 = 启动直接崩

方式 B：运行期 dlopen
   FPlatformProcess::GetDllHandle("libdpvr.so") + dlsym 取函数指针
   优点：.so 不在也能降级，不依赖完整头文件
   缺点：签名写错要跑起来才知道
```

参考工程两种都有：相机走 B（`PICO_EnterpriseInterfaceWrapper.cpp:49` + `:20` 的 dlsym），另一个 `.so` 走 A（`PICOEnterprise.Build.cs:42-44`）。

---

## 5. `aar` / `jar`：C++ 碰不到，必须过 Java

`aar` = Android 的 Java 库压缩包（`classes.jar` + 清单 + 资源 + 有时附带 `.so`）。

```
C++  ──JNI──▶  你写的 Java 封装  ──import──▶  厂商 aar 里的类  ──▶  他们的 .so
```

**C++ 不能 import Java 类**，所以只要 SDK 是 aar，就必然多一层 Java 封装。

参考工程：PICO 的 tobservice 是 maven 上的 aar，UPL 里 `implementation 'com.picoxr.tobservice:tobservicelib:4.3.20'`，Java 里 bind 服务，C++ 再 JNI 调那层 Java。

---

## 6. 三条路对照（记住这一张表就够）

| 你要的能力 | 底层路径 | 写 Java | 写 XML |
|---|---|---|---|
| 电量、音量、温度、网络、设备 ID | 引擎 C++ 接口 | 否 | 否 |
| 屏幕亮灭、亮度、跨 App 拉起 | 自己的 Java | 是 | 是（只装 Java 那张纸条） |
| 大鹏手柄、企业管控 | 他们的 `.so` | 否 | 是（一行 copyFile） |

---

## 7. 中途纠正过的三个判断（留痕）

讨论过程中有三处一开始说错，后面查源码改掉了，记下来避免以后再错：

| 一开始的判断 | 查证后的结论 | 证据 |
|---|---|---|
| 非 Java 线程调 `GetJavaEnv()` 会拿到空指针 | 错。它会自动 `AttachCurrentThread`；真正会拿到空指针的是 `JNI_OnLoad` 之前或 Activity 没挂上 | `AndroidJavaEnv.cpp:131` |
| 头显电量要自己写 Java 读 BatteryManager | 错。引擎自带 `FPlatformMisc::GetBatteryLevel()`，参考工程那 45 行是白写的 | `AndroidPlatformMisc.cpp:2835`、`AndroidPlatformMisc.h:145` |
| 要做"厂商接口 + PICO/大鹏双后端" | 确认只支持大鹏后，双后端是过度设计，直接一个 `FDPVRBackend` | 本工程决策 |

---

## 8. 结论

1. UE 没有安卓系统，只有一层 JNI 桥。
2. XML 是打包载体，不是代码；它只往 APK 里搬 Java、权限、依赖。
3. 有 Java API 的能力都能用 JNI 做；只有 native 的（厂商 `.so`）不需要 JNI，直接 C++。
4. `.so` 是动态库，用法像 lib，但必须打进 APK 且依赖补齐。
5. 大鹏的写法取决于他们给什么：`.so` → 纯 C++；`aar` → 多一层 Java。**先拆包看目录，别猜。**
