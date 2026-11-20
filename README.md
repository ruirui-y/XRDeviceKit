# XR Device Kit

面向大鹏 DPVR 头显的 UE5 设备能力插件。把 Android 系统能力（电量、屏幕、音量、存储、应用信息）封装成蓝图可直接拖用的节点，调用方不需要写 C++，也不需要知道底层走的是引擎接口还是 JNI。

- **67 个节点**，按能力分 11 类，蓝图里搜 `XRDeviceKit` 就能全部找到
- **失败不抛异常**，返回固定默认值，调用方不用做保护
- **Win64 编辑器下返回默认值**，没有安卓设备也能正常打开工程、跑蓝图
- 已在 **DPVR P1Max（Android 12）** 上逐项实测，值可直接和头显设置页对上

## 能力一览

| 分类 | 数量 | 内容 |
|---|---|---|
| 设备 | 19 | 电量、充电状态、电池/CPU 温度、电池电流与健康、设备 ID、系统版本、厂商型号、ABI、磁盘空间 |
| 屏幕 | 13 | 亮灭、亮度读写、屏幕方向、分辨率、DPI、密度、刷新率、息屏超时、深色模式 |
| 网络 | 3 | 网络类型、WiFi 连接状态、本机 IP |
| 音频 | 4 | 音量读写（0~100）、铃声模式、耳机插入 |
| 存储 | 4 | 内部/外部存储的剩余与总量 |
| 应用 | 8 | 包名、版本、安装时间、应用名、是否安装、拉起 App |
| 系统 | 8 | 开机时长、时区、语言与语言列表、工程版本、剪贴板读写、系统分享 |
| 振动 | 2 | 开始/停止振动 |
| 性能 | 2 | 渲染帧率、帧耗时 |
| 资源统计 | 4 | CPU/内存/线程数快照、可配间隔的日志打印 |
| 硬件信息 | 1 | 厂商型号、CPU、GPU、内存、屏幕、系统版本 |
| 事件 | 2 | 屏幕亮灭、网络连通（蓝图 Bind，不用轮询） |

完整的节点说明、返回值和实测值见 **[API 参考](docs/10-API参考.md)**。

## 安装

1. 把 `Plugins/XRDeviceKit` 整个目录拷进你的工程 `Plugins/` 下
2. 打开 `uproject`，在 **Edit → Plugins → Device** 里启用 **XR Device Kit**
3. 重启编辑器，蓝图里右键搜 `XRDeviceKit`

> 插件自带 Java 模块和 Android 打包补丁，打包时会自动处理，不需要手工改 gradle 或清单。

## 环境要求

| 项 | 版本 |
|---|---|
| Unreal Engine | 5.5 |
| 目标平台 | Android（arm64-v8a） |
| 开发平台 | Win64（只用来编辑和调试，功能返回默认值） |
| 目标设备 | 大鹏 DPVR 头显 |

## 本仓库结构

```
XRDeviceKitDemo/                 UE 工程，用来开发、打包、真机验证
├─ Plugins/XRDeviceKit/          插件本体，交付的就是这个目录
│  ├─ XRDeviceKit.uplugin
│  ├─ Source/XRDeviceKit/        C++ 源码
│  └─ JavaLibs/xrbridge/         插件自带的 Java 模块
├─ docs/                         文档（API 参考 + 设计记录 + 实测日志）
└─ Tools/verify_apk.ps1          打包产物检查脚本
```

**只想用插件的话，只需要 `Plugins/XRDeviceKit` 这一个目录。**

## 开发本仓库

1. 右键 `XRDeviceKitDemo.uproject` → **Generate Visual Studio project files**
2. 编译 `Development Editor | Win64`
3. 打开编辑器验证插件已启用

`.sln`、`Binaries/`、`Intermediate/`、`Saved/` 都在 `.gitignore` 里，新克隆的仓库必须重新生成一次。

进度、测试命令和待办见 **[开发日志](docs/11-开发日志.md)**。

## 文档

**给使用方**

| 文档 | 内容 |
|---|---|
| [docs/10-API参考.md](docs/10-API参考.md) | 67 个节点的说明、返回值、平台差异、实测值 |
| [docs/11-开发日志.md](docs/11-开发日志.md) | 进度、测试方法、待办 |
| [docs/12-资源统计与硬件信息.md](docs/12-资源统计与硬件信息.md) | 资源采样怎么做的、哪些字段在 P1Max 拿得到 |
| [docs/13-事件-屏幕与网络.md](docs/13-事件-屏幕与网络.md) | 事件机制：native 反向回调、线程切换、符号验证 |
| [docs/14-插件独立打包验证.md](docs/14-插件独立打包验证.md) | BuildPlugin 怎么打、JavaLibs 差点丢包、包内容清单 |
| [docs/15-大鹏SDK摸底.md](docs/15-大鹏SDK摸底.md) | QVR300 包里有什么、GSXR 为什么跑不起来、手柄电量在哪 |

**内部记录**

| 文档 | 内容 |
|---|---|
| [docs/00-UE安卓层与JNI原理.md](docs/00-UE安卓层与JNI原理.md) | UE 安卓层怎么工作：XML 注入、JNI、`.so` 与 `aar` |
| [docs/01-插件范围与阶段规划.md](docs/01-插件范围与阶段规划.md) | 范围、三档拆分、架构、待大鹏确认的问题 |
| [docs/02-P0-插件骨架与三个设备API.md](docs/02-P0-插件骨架与三个设备API.md) | P0：分层与编译踩坑 |
| [docs/03-P1第一批-设备信息与系统能力.md](docs/03-P1第一批-设备信息与系统能力.md) | P1-1：28 个引擎节点 |
| [docs/04-P1第二批-Java模块与独立Java类.md](docs/04-P1第二批-Java模块与独立Java类.md) | P1-2：独立 Java 模块怎么搭 |
| [docs/05-P1第三批-应用信息与屏幕存储.md](docs/05-P1第三批-应用信息与屏幕存储.md) | P1-3：15 个 Java 能力 |
| [docs/06-P1第四批-构建信息与音量.md](docs/06-P1第四批-构建信息与音量.md) | P1-4：12 个节点与平台限制 |
| [docs/07-测试-契约与冒烟.md](docs/07-测试-契约与冒烟.md) | 测试三件套 |
| [docs/08-真机冒烟-P1Max实测.md](docs/08-真机冒烟-P1Max实测.md) | P1Max 实测与问题定位 |
| [docs/09-实测对照表.md](docs/09-实测对照表.md) | 逐项对照表 |

## 仓库信息

**About**

> UE5 头显设备能力插件：67 个蓝图节点覆盖设备信息、屏幕、网络、音量、存储与应用管控，底层对接 Android 系统接口与大鹏 DPVR SDK。

**Topics**

`unreal-engine` `ue5` `android` `dpvr` `vr` `plugin` `jni`

## 许可

尚未确定。
