# P1 第二批：独立 Java 模块打通

> 日期：2026-11-18
> 前置：[03-P1第一批-设备信息与系统能力.md](03-P1第一批-设备信息与系统能力.md)

---

## 1. 这一阶段要做什么

把 Java 代码从 `GameActivity` 里搬出来，改成**独立的 Java 模块**。以后 40 多个需要 Java 的 API 全部写在那里，`GameActivity` 不再继续膨胀。

本轮只做两个方法，目的是把**打包链路**验证通，不是堆功能：

| 方法 | 作用 |
|---|---|
| `XRBridge.Init(Context)` | 启动时把 Application Context 存起来 |
| `XRBridge.GetPackageName()` | 返回运行中的包名，验证 C++ 真的调到了 Java |

**验收方式**：不看真机，只看打包产物——模块有没有被 gradle 认出来、类有没有进 dex。

---

## 2. 怎么做的

### 2.1 目录

```
Plugins/XRDeviceKit/JavaLibs/xrbridge/
├─ additions.gradle        追加到 UAT 生成的 build.gradle 后面（只写 namespace）
├─ AndroidManifest.xml     空清单，namespace 在 gradle 里声明
├─ project.properties      android.library=true，标记这是个库模块
└─ src/main/java/com/xrdevicekit/bridge/
   └─ XRBridge.java        两个静态方法
```

**为什么用 `src/main/java`**：这是 Android Gradle Plugin 的默认源码目录。引擎自带的 `permission_library` 用的是老式 `src/`，靠老工具链；我们走默认目录，不额外声明 `sourceSets`。

### 2.2 XML 里加了三块

```xml
<!-- ① 整个模块拷进打包目录：加文件不用改这里，加模块才要加一条 -->
<prebuildCopies>
    <copyDir src="$S(PluginDir)/../../../JavaLibs/xrbridge"
             dst="$S(BuildDir)/JavaLibs/xrbridge"/>
</prebuildCopies>

<!-- ② 让 app 依赖这个模块 -->
<buildGradleAdditions>
    <insert>
dependencies {
    implementation project(':xrbridge')
}
    </insert>
</buildGradleAdditions>

<!-- ③ 启动时把 Context 交过去 -->
<gameActivityOnCreateAdditions>
    <insert><![CDATA[
        try { XRBridge.Init(getApplicationContext()); }
        catch (Throwable t) { Log.debug("XRDeviceKit: XRBridge.Init failed: " + t.toString()); }
    ]]></insert>
</gameActivityOnCreateAdditions>
```

第 ③ 块只加**一行有效代码**，而且包在 try/catch 里：万一模块没打进包，启动时不至于直接崩。

### 2.3 C++ 侧

新增 `Private/XRDeviceKitJava.h/.cpp`：找一次类（`FindJavaClassGlobalRef`）缓存成全局引用，之后每次只调静态方法。

```
Platform::GetPackageName()
  └─ Java::GetPackageName()
      ├─ FindJavaClassGlobalRef("com/xrdevicekit/bridge/XRBridge")
      ├─ GetStaticMethodID(..., "GetPackageName", "()Ljava/lang/String;")
      └─ CallStaticObjectMethod → FJavaHelper::FStringFromLocalRef
```

---

## 3. 写完为什么不是理想效果

三次失败，前两次是配置问题，第三次是我自己写错语法。

### 失败 1：模块压根没拷过去

```
FAILURE: Build failed with an exception.
* Where:
Script 'V:\app\buildAdditions.gradle' line: 26
* What went wrong:
A problem occurred evaluating script.
> Project with path ':xrbridge' could not be found in project ':app'.
```

### 失败 2：注释被 UAT 读成乱码

生成的模块 `build.gradle` 末尾：

```
// UAT 浼氫负鏈ā鍧楃敓鎴?build.gradle锛氱劧鍚庢妸杩欎唤鏂囦欢杩藉姞杩涘幓銆?
```

我写在 `additions.gradle` 里的中文注释，被 UAT 拼进生成文件时整段花了。

### 失败 3：gradle 文件里用了 `#` 注释

我把 `additions.gradle` 的注释从中文改成英文时，顺手写成了 `# ...`。构建直接失败，**错误文本没抓全**（命令只留了末尾 10 行，看到 `BUILD FAILED in 12s` 就没了）。原因很明确：gradle 是 Groovy，注释只认 `//`。

---

## 4. 怎么定位的

**失败 1**：先不去猜 gradle，直接看打包目录里有没有模块：

```powershell
Get-ChildItem Intermediate\Android\arm64\JavaLibs
  downloader_library
  permission_library        ← 没有 xrbridge
```

目录不存在，说明 `copyDir` 的源路径算错了。数一下层级：

```
XML 位置：Plugins/XRDeviceKit/Source/XRDeviceKit/Android/XRDeviceKit_APL.xml
$S(PluginDir) = .../Source/XRDeviceKit/Android
  ../            → .../Source/XRDeviceKit
  ../../         → .../Source          ← 我原来写的是这个，错
  ../../../      → .../XRDeviceKit     ← 插件根，才对
```

**失败 2**：看生成文件就能发现——`Intermediate/Android/arm64/gradle/xrbridge/build.gradle` 里 my 的中文全花了。UAT 是按 ANSI 读写的，UTF-8 中文必花。

**失败 3**：错误没抓到，但改动很小（只有注释行），回退到 `//` 立刻通过，因果关系清楚。

---

## 5. 怎么解决的

| 问题 | 改法 |
|---|---|
| 路径少一级 | `../../` → `../../../` |
| additions.gradle 有中文 | 全部改英文，并在文件头写一句"这个文件必须纯 ASCII" |
| `#` 注释 | 改成 `//` |

另外补了一个 `project.properties`（`android.library=true`），跟引擎自带的 `permission_library`、`downloader_library` 保持一致。

---

## 6. 最终效果

### 三项检查全部通过

**① gradle 认出了模块**

```
Intermediate/Android/arm64/gradle/settings.gradle
  include ':app'
  include ':downloader_library'
  include ':permission_library'
  include ':xrbridge'        ← 自动生成，不用手写
```

**② 文件确实拷过去了**

```
Intermediate/Android/arm64/JavaLibs/xrbridge/
  additions.gradle
  AndroidManifest.xml
  project.properties
  src/main/java/com/xrdevicekit/bridge/XRBridge.java
```

**③ 类进了 APK 的 dex**

```
classes3.dex    com/xrdevicekit/bridge/XRBridge | GetPackageName
classes14.dex   com/xrdevicekit/bridge/XRBridge | XRDeviceKit_IsScreenOn
```

第一行是我们新模块，第二行是 P0 注入进 GameActivity 的老方法——两套机制并存，互不干扰。

### 注入点

生成的 `GameActivity.java`：

```
L229:  import com.xrdevicekit.bridge.XRBridge;
L3704: XRBridge.Init(getApplicationContext());
```

### 验收表

| 项 | 状态 | 证据 |
|---|---|---|
| Win64 编译 | ✅ | 7 action，12.39 秒，exit 0 |
| Android 编译 | ✅ | BUILD SUCCESSFUL in 19s，exit 0 |
| `include ':xrbridge'` 自动生成 | ✅ | settings.gradle 第 5 行 |
| 模块文件拷入打包目录 | ✅ | JavaLibs/xrbridge 四个文件 |
| 类进 dex | ✅ | classes3.dex |
| 生成的模块 build.gradle 无乱码 | ✅ | 改 ASCII 后复查 |
| 真机拿到包名 | ⬜ | 待设备 |

### 这条链路打通后，加 API 的成本

```
加一个 Java 能力 = 改两个文件
  XRBridge.java        加一个 static 方法
  XRDeviceKitJava.cpp  加一段查方法 + 调用
XML 不用动，gradle 不用动，目录不用动
```
