# XR Device Kit

面向大鹏 DPVR 头显的 UE5 设备能力插件。把 Android 系统接口（电量、音量、亮度、屏幕状态、应用管控）和 DPVR SDK（手柄、投屏、企业管控）统一封装成蓝图可直接拖用的 UFUNCTION 节点，调用方不需要知道底层是 JNI 还是 native `.so`。

> 当前状态：**P1 第四批 + 测试三件套完成**。56 个蓝图节点；契约测试通过（exit 0）；APK 符号检查全过；真机冒烟待设备。

## 目录结构

```
XRDeviceKitDemo/                 UE 工程（只用来开发、打包、跑真机验证）
├─ Plugins/XRDeviceKit/          插件本体，最后交付的就是这个目录
│  ├─ XRDeviceKit.uplugin
│  └─ Source/XRDeviceKit/
│     ├─ XRDeviceKit.Build.cs
│     ├─ Public/
│     │  ├─ XRDeviceKit.h              模块头
│     │  └─ XRDeviceKitLibrary.h       蓝图节点（对外唯一入口）
│     ├─ Private/
│     │  ├─ XRDeviceKit.cpp
│     │  ├─ XRDeviceKitLibrary.cpp     转发，不含平台分支
│     │  ├─ XRDeviceKitPlatform.*      平台层，唯一有 #if PLATFORM_ANDROID 的地方
│     │  ├─ XRDeviceKitJni.*           JNI 层，调注入进 GameActivity 的方法
│     │  └─ XRDeviceKitJava.*          JNI 层，调独立 Java 模块的静态方法
│     └─ Android/XRDeviceKit_APL.xml   打包补丁：注入 Java、拷 Java 模块、改 gradle
├─ Plugins/XRDeviceKit/JavaLibs/xrbridge/   独立 Java 模块（XRBridge.java）
├─ docs/                         设计文档
└─ Source/XRDeviceKitDemo/       工程主模块
```

## 环境要求

| 项 | 版本 |
|---|---|
| Unreal Engine | 5.5 |
| Visual Studio | 2022（含 C++ 游戏开发工作负载） |
| Android SDK / NDK | 随 UE 5.5 安装 |
| 目标设备 | 大鹏 DPVR 头显，arm64-v8a |

## 快速开始

1. 右键 `XRDeviceKitDemo.uproject` → **Generate Visual Studio project files**
2. 打开生成的 `XRDeviceKitDemo.sln`，编译 `Development Editor | Win64`
3. 双击 `XRDeviceKitDemo.uproject` 打开编辑器
4. 插件在 **Edit → Plugins → Device** 分类下，已默认启用

> `.sln` 和 `Binaries/`、`Intermediate/`、`Saved/` 都在 `.gitignore` 里，新克隆的仓库必须重新生成一次。

## 测试

三层，各自负责不同的问题：

```powershell
# 1. 契约测试：56 个节点的默认值与注册数量（编辑器，10 秒）
& "C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" `
  "C:\Users\Mars\UE5Project\XRDeviceKitDemo\XRDeviceKitDemo.uproject" `
  -ExecCmds="Automation RunTests XRDeviceKit;Quit" -unattended -nop4 -nosplash -nullrhi -log
# 结果看 Saved/Logs/XRDeviceKitDemo.log，不看 stdout，搜 "Test Completed"

# 2. APK 检查：符号有没有进包（打包后，2 秒）
pwsh Tools/verify_apk.ps1

# 3. 真机冒烟：Config/DefaultGame.ini 里加 [XRDeviceKit] SmokeTestOnStartup=True，装包后
adb logcat -s UE:V | findstr "SMOKE|"
```

## 开发计划

| 阶段 | 内容 | 状态 |
|---|---|---|
| P0 | 插件骨架 + 电量、屏幕状态、窗口亮度三个 API | 完成，安卓包已验证注入生效 |
| P1 | 一档：无权限、纯标准 Android 能力 | 四批共 56 个节点；Java 模块承载 27 个能力 |
| P2 | 大鹏 SDK 对接（等 SDK） | 阻塞中 |
| P3 | 二档：需要系统权限的能力 | 未开始 |

## 文档

| 文档 | 内容 |
|---|---|
| [docs/00-UE安卓层与JNI原理.md](docs/00-UE安卓层与JNI原理.md) | UE 安卓层到底怎么工作：XML 注入、JNI、`.so` 与 `aar` 的区别，附参考工程里的证据行号 |
| [docs/01-插件范围与阶段规划.md](docs/01-插件范围与阶段规划.md) | 范围、API 三档拆分、架构、阶段验收标准、待大鹏确认的问题 |
| [docs/02-P0-插件骨架与三个设备API.md](docs/02-P0-插件骨架与三个设备API.md) | P0 阶段记录：分层、四个决策、编码踩坑、编译实测结果 |
| [docs/03-P1第一批-设备信息与系统能力.md](docs/03-P1第一批-设备信息与系统能力.md) | P1 第一批：28 个节点清单、枚举镜像决策、链接错误定位、安卓编译实测 |
| [docs/04-P1第二批-Java模块与独立Java类.md](docs/04-P1第二批-Java模块与独立Java类.md) | P1 第二批：独立 Java 模块怎么搭、拷贝路径算错、gradle 注释踩坑、dex 验证 |
| [docs/05-P1第三批-应用信息与屏幕存储.md](docs/05-P1第三批-应用信息与屏幕存储.md) | P1 第三批：15 个 Java 能力、通用调用器、UP-TO-DATE 疑云的排除 |
| [docs/06-P1第四批-构建信息与音量.md](docs/06-P1第四批-构建信息与音量.md) | P1 第四批：12 个节点、包可见性限制、亮度只读的取舍 |
| [docs/07-测试-契约与冒烟.md](docs/07-测试-契约与冒烟.md) | 测试三件套：契约测试、真机冒烟、APK 检查脚本 |

## 仓库信息

**About（GitHub 描述）**

> UE5 头显设备能力插件：把电量、屏幕、音量、手柄、投屏等系统与厂商能力封装成蓝图节点，底层对接大鹏 DPVR SDK。

**Topics**

`unreal-engine` `ue5` `android` `dpvr` `vr` `plugin` `jni`

## 许可

尚未确定。
