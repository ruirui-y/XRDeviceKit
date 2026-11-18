// 蓝图唯一入口：所有设备能力都从这里暴露成节点。
// 这里只做转发，不写平台分支、不碰 JNI，真正的实现在 Private/XRDeviceKitPlatform.cpp。
//
// 约定：所有函数的失败返回值写在各自注释里，调用方不需要 try/catch。

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "XRDeviceKitTypes.h"
#include "XRDeviceKitLibrary.generated.h"

UCLASS()
class XRDEVICEKIT_API UXRDeviceKitLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ---------------- 设备 / 电源 ----------------

	// 头显当前电量 0~100。非 Android 平台返回 -1。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Device")
	static int32 GetBatteryLevel();

	// 是否正在充电（含已充满）。非 Android 平台返回 false。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Device")
	static bool IsCharging();

	// 系统是否处于省电模式。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Device")
	static bool IsLowPowerMode();

	// 电池温度，摄氏度。拿不到返回 -1。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Device")
	static float GetBatteryTemperature();

	// CPU 温度，摄氏度。拿不到返回 -1。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Device")
	static float GetCPUTemperature();

	// 设备唯一 ID，卸载重装会变。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Device")
	static FString GetDeviceId();

	// 系统版本号字符串，例如 "12"。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Device")
	static FString GetOSVersionString();

	// 安卓主版本号，例如 12。非 Android 平台返回 -1。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Device")
	static int32 GetAndroidMajorVersion();

	// CPU 型号字符串。拿不到返回空串。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Device")
	static FString GetCPUBrand();

	// 物理核心数。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Device")
	static int32 GetCoreCount();

	// 系统默认语言，例如 "zh-Hans-CN"。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Device")
	static FString GetDefaultLocale();

	// 指定路径所在分区的剩余空间，单位 MB。拿不到返回 -1。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Device")
	static int64 GetDiskFreeMegabytes(const FString& Path);

	// 指定路径所在分区的总空间，单位 MB。拿不到返回 -1。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Device")
	static int64 GetDiskTotalMegabytes(const FString& Path);

	// ---------------- 显示 ----------------

	// 屏幕当前是否亮着。只区分亮和灭，不代表头显有没有被戴上。
	// 非 Android 平台返回 true，避免编辑器里调试用的界面全灭。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Display")
	static bool IsScreenOn();

	// 本 App 窗口的亮度 0~1；返回 -1 表示当前跟随系统亮度。
	// 只影响本 App，不改整机亮度。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Display")
	static float GetBrightness();

	// 设置本 App 窗口亮度，入参 0~1，越界会被夹回来。
	// 返回 false 表示平台不支持或设置失败。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Display")
	static bool SetBrightness(float Brightness);

	// 当前屏幕方向。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Display")
	static EXRDeviceScreenOrientation GetScreenOrientation();

	// 设置屏幕方向。返回 false 表示平台不支持。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Display")
	static bool SetScreenOrientation(EXRDeviceScreenOrientation Orientation);

	// ---------------- 网络 ----------------

	// 当前网络类型。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Network")
	static EXRDeviceNetworkType GetNetworkType();

	// 是否连着 WiFi。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Network")
	static bool HasWiFiConnection();

	// ---------------- 音频 ----------------

	// 媒体音量档位 0~15（安卓的档位数不固定，以实际设备为准）。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Audio")
	static int32 GetDeviceVolume();

	// 是否插着耳机。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Audio")
	static bool IsHeadphonesPluggedIn();

	// ---------------- 应用信息 ----------------

	// 版本名，例如 "1.0"。非 Android 平台返回空串。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|App")
	static FString GetVersionName();

	// 版本号（构建号）。非 Android 平台返回 -1。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|App")
	static int64 GetVersionCode();

	// 指定包名是否已安装。Android 11+ 受包可见性限制：没在清单 <queries> 里声明的包会返回 false。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|App")
	static bool IsAppInstalled(const FString& PackageName);

	// ---------------- 屏幕尺寸 ----------------

	// 屏幕宽度（像素）。非 Android 平台返回 0。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Display")
	static int32 GetScreenWidth();

	// 屏幕高度（像素）。非 Android 平台返回 0。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Display")
	static int32 GetScreenHeight();

	// 屏幕密度（dpi），例如 320。非 Android 平台返回 0。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Display")
	static int32 GetScreenDpi();

	// 密度倍率，相对 160dpi 的倍数，例如 2.0。非 Android 平台返回 1.0。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Display")
	static float GetScreenDensity();

	// 屏幕刷新率 Hz，例如 72 或 90。拿不到返回 0。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Display")
	static float GetScreenRefreshRate();

	// ---------------- 存储 ----------------

	// 内部存储剩余空间（MB）。拿不到返回 -1。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Storage")
	static int64 GetInternalFreeMegabytes();

	// 内部存储总空间（MB）。拿不到返回 -1。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Storage")
	static int64 GetInternalTotalMegabytes();

	// 外部存储剩余空间（MB）。拿不到返回 -1。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Storage")
	static int64 GetExternalFreeMegabytes();

	// 外部存储总空间（MB）。拿不到返回 -1。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Storage")
	static int64 GetExternalTotalMegabytes();

	// ---------------- 系统 ----------------

	// 运行中的包名，例如 com.YourCompany.XRDeviceKitDemo。非 Android 平台返回空串。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|System")
	static FString GetPackageName();

	// 开机时长（秒），含深睡眠时间。非 Android 平台返回 -1。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|System")
	static int64 GetUptimeSeconds();

	// 时区 ID，例如 "Asia/Shanghai"。非 Android 平台返回空串。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|System")
	static FString GetTimeZoneId();

	// 语言标签，例如 "zh-CN"。非 Android 平台返回空串。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|System")
	static FString GetLanguageTag();

	// 把文本写入系统剪贴板。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|System")
	static void CopyToClipboard(const FString& Text);

	// 读系统剪贴板，拿不到返回空串。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|System")
	static FString PasteFromClipboard();

	// 工程版本号（Project Settings → Project → Version）。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|System")
	static FString GetProjectVersion();

	// 调系统分享面板。非 Android 平台是空实现。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|System")
	static void ShareURL(const FString& URL, const FString& Description);

	// ---------------- 设备构建信息 ----------------

	// 厂商，例如 "DPVR"。非 Android 平台返回空串。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Device")
	static FString GetManufacturer();

	// 型号，例如 "DPVR E4"。非 Android 平台返回空串。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Device")
	static FString GetModel();

	// 设备代号（Build.DEVICE）。非 Android 平台返回空串。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Device")
	static FString GetDeviceName();

	// 支持的 ABI，逗号分隔，例如 "arm64-v8a,armeabi-v7a"。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Device")
	static FString GetSupportedABIs();

	// ---------------- 应用信息（补充） ----------------

	// 首次安装时间（毫秒时间戳）。非 Android 平台返回 -1。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|App")
	static int64 GetFirstInstallTimeMs();

	// 最后更新时间（毫秒时间戳）。非 Android 平台返回 -1。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|App")
	static int64 GetLastUpdateTimeMs();

	// 应用显示名（桌面图标下面那个名字）。非 Android 平台返回空串。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|App")
	static FString GetAppLabel();

	// 拉起指定包名的 App。受 Android 11 包可见性限制：没在清单 <queries> 里声明的包会返回 false。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|App")
	static bool StartApp(const FString& PackageName);

	// ---------------- 音频（补充） ----------------

	// 媒体音量最大档位，通常是 15。拿不到返回 0。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Audio")
	static int32 GetMaxVolume();

	// 设置媒体音量档位。返回 false 表示平台不支持或失败。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Audio")
	static bool SetVolume(int32 Level);

	// ---------------- 显示（补充） ----------------

	// 系统亮度（平台自己的刻度，通常是 0~255）。读取不需要权限，写入需要 WRITE_SETTINGS，所以没有 setter。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Display")
	static int32 GetSystemBrightness();

	// 系统是否处于深色模式。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Display")
	static bool IsNightMode();

	// ---------------- 振动 ----------------

	// 触发一次振动。Strength：0 轻 / 1 中 / 2 重，越界按中处理。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Haptics")
	static void StartHaptics(int32 Strength);

	// 停止并释放振动资源，配对 StartHaptics 使用。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Haptics")
	static void StopHaptics();
};
