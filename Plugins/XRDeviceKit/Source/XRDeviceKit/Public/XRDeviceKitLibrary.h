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

	// ---------------- 系统 ----------------

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

	// ---------------- 振动 ----------------

	// 触发一次振动。Strength：0 轻 / 1 中 / 2 重，越界按中处理。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Haptics")
	static void StartHaptics(int32 Strength);

	// 停止并释放振动资源，配对 StartHaptics 使用。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Haptics")
	static void StopHaptics();
};
