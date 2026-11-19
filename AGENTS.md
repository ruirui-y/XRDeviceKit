# AGENTS.md — XRDeviceKitDemo

> 本项目级约定。全局规则见 `~/.dsh/AGENTS.md`，两份冲突时以本项目为准。

## 1. 这是什么

UE 5.5 工程 + 一个插件。目标：把大鹏 DPVR 头显的设备能力封装成蓝图可直接拖用的节点。
**工程只是开发和验证的壳，交付物是 `Plugins/XRDeviceKit` 整个目录。**

| 项 | 值 |
|---|---|
| 引擎 | UE 5.5 |
| 工程 / 主模块 | `XRDeviceKitDemo`（必须同名） |
| 插件 / 插件模块 | `XRDeviceKit`（不能与工程模块重名） |
| 厂商 | 只支持大鹏 DPVR |
| 交付形态 | 整个 `Plugins/XRDeviceKit` 目录，可拷进任意 UE 5.5 工程 |

## 2. 目录约定

```
Plugins/XRDeviceKit/
├─ XRDeviceKit.uplugin
├─ Source/XRDeviceKit/
│  ├─ XRDeviceKit.Build.cs
│  ├─ Public/        ← 蓝图库、对外类型；只放 UE 反射能看懂的东西
│  ├─ Private/       ← 后端实现、JNI 封装；外部不可见
│  └─ Android/       ← UPL XML（xx_APL.xml），只在打 Android 包时用
└─ JavaLibs/         ← 独立 Java 模块；必须在 Config/FilterPlugin.ini 里登记才会打进插件包
```

## 3. 架构约定（违反要回退）

1. **蓝图库只做门面**。`U*Library` 里不许出现 JNI 调用、`dlopen`、`#include <dlfcn.h>`。
2. **厂商相关代码全部集中在 `FDPVRBackend`**。换厂商只改这一个文件，蓝图签名不动。
3. **注入 `GameActivity` 的 Java 只放"必须拿 Activity"的方法**（拉起 App、注册广播）。纯查询走独立 Java 类，避免重蹈参考工程那份 3700 行。
4. **所有 Android 代码必须 `#if PLATFORM_ANDROID` 包住**；Win64 下返回默认值（`-1` / `false` / 空串），**不许断言、不许崩**——编辑器里要能正常打开工程。
5. **不做"厂商接口 + 多后端"抽象**。只有一家后端时不引入虚接口，等真要接第二家再抽。

## 4. 编码约定

- 注释按全局规则：口语化中文；函数头顶 1~2 行说业务；成员变量行尾注释；`if` 分支注释写在 `if` 那一行上面。
- 蓝图节点命名：`GetXxx` / `SetXxx` / `IsXxx` / `StartXxx` / `StopXxx`。
- `UFUNCTION` 的 `Category` 一级固定 `XRDeviceKit`，二级按能力域：`XRDeviceKit|Device`、`XRDeviceKit|Display`、`XRDeviceKit|App`、`XRDeviceKit|Controller`。
- 蓝图库不进 Tick、不阻塞等待。异步结果一律用 `DECLARE_DYNAMIC_MULTICAST_DELEGATE`，或者挂 `UGameInstanceSubsystem`。
- JNI 的 `jmethodID` / `jclass` 集中缓存在一个文件里，不许散落在各个业务函数里。
- 新增 API 必须同时给出"Win64 下返回什么"，写在函数头顶注释里。

## 5. 构建与验证

编译由使用者执行，AI 只交付代码和命令。

```powershell
# 1. 编译编辑器
& "C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\Build.bat" `
  XRDeviceKitDemoEditor Win64 Development `
  -Project="C:\Users\Mars\UE5Project\XRDeviceKitDemo\XRDeviceKitDemo.uproject" -WaitMutex

# 2. 打包插件（验证插件能脱离工程独立编译）
& "C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin `
  -Plugin="C:\Users\Mars\UE5Project\XRDeviceKitDemo\Plugins\XRDeviceKit\XRDeviceKit.uplugin" `
  -Package="C:\Users\Mars\UE5Project\XRDeviceKitDemo\Pack\XRDeviceKit" `
  -TargetPlatforms=Win64+Android

# 3. 打安卓包（真机验证用）
& "C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun `
  -project="C:\Users\Mars\UE5Project\XRDeviceKitDemo\XRDeviceKitDemo.uproject" `
  -platform=Android -cook -build -stage -package -archive `
  -archivedirectory="C:\Users\Mars\UE5Project\XRDeviceKitDemo\Pack"
```

**每个阶段的验收必须真机跑过**。Win64 编译通过只说明代码没错，不说明 JNI 通。

## 6. 提交规范

格式：`type(scope): 一句话说清改了什么`，空行，正文分点写「改了什么 / 为什么 / 怎么验证」。

- `type` 只用：`feat` `fix` `refactor` `style` `docs` `test` `chore`
- `scope` 小写模块名：`plugin` `jni` `java` `upl` `docs` `agents` `build`
- **一个阶段一次提交**，不攒到最后
- **代码提交前必须有"编译通过"的确认**；纯文档/配置提交可以直接提
- **阶段文档不要写"学到什么"或经验总结类小节**，只保留过程、原始报错、实测数据、验收状态
- **docs 下的文件名统一中文**，数字前缀、不带空格：`02-P0-插件骨架与三个设备API.md`

## 7. 不要做的事

| 不做 | 原因 |
|---|---|
| 引入 PICO 插件或 tobservice 依赖 | 只对接大鹏 |
| 提交 `Binaries/` `Intermediate/` `Saved/` `DerivedDataCache/` `*.sln` | 已进 `.gitignore`，可重新生成 |
| 在插件里写死工程名、包名 | 插件要能拷到别的工程 |
| 往 `GameActivity` 里堆几十个 Java 方法 | 会变成参考工程那样 350 KB 的 Java 文件 |
| 用 `android.util.Log` 当唯一排查手段 | 先看 `UE_LOG`，再看 `adb logcat` |
