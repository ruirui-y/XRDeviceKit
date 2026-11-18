# XR Device Kit

面向大鹏 DPVR 头显的 UE5 设备能力插件。把 Android 系统接口（电量、音量、亮度、屏幕状态、应用管控）和 DPVR SDK（手柄、投屏、企业管控）统一封装成蓝图可直接拖用的 UFUNCTION 节点，调用方不需要知道底层是 JNI 还是 native `.so`。

> 当前状态：**P1 第二批完成**。29 个蓝图节点；独立 Java 模块（`JavaLibs/xrbridge`）已打通，新增 Java 能力不再往 GameActivity 里堆；真机取值待验证。

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

## 开发计划

| 阶段 | 内容 | 状态 |
|---|---|---|
| P0 | 插件骨架 + 电量、屏幕状态、窗口亮度三个 API | 完成，安卓包已验证注入生效 |
| P1 | 一档：无权限、纯标准 Android 能力 | 第一批 28 个完成；第二批 Java 模块打通（+1 节点） |
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

## 仓库信息

**About（GitHub 描述）**

> UE5 头显设备能力插件：把电量、屏幕、音量、手柄、投屏等系统与厂商能力封装成蓝图节点，底层对接大鹏 DPVR SDK。

**Topics**

`unreal-engine` `ue5` `android` `dpvr` `vr` `plugin` `jni`

## 许可

尚未确定。
