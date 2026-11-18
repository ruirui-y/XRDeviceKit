// 蓝图唯一入口：所有设备能力都从这里暴露成节点。
// 这里只做转发，不写平台分支、不碰 JNI，真正的实现在 Private/XRDeviceKitPlatform.cpp。

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "XRDeviceKitLibrary.generated.h"

UCLASS()
class XRDEVICEKIT_API UXRDeviceKitLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 头显当前电量 0~100。非 Android 平台返回 -1。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Device")
	static int32 GetBatteryLevel();

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
};
