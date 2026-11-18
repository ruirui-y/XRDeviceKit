// 蓝图能看懂的枚举。
// 引擎的 EDeviceScreenOrientation / ENetworkConnectionType 不是 UENUM，蓝图用不了，
// 这里镜像一份；转换在平台层用 switch 显式映射，不靠枚举顺序。

#pragma once

#include "CoreMinimal.h"
#include "XRDeviceKitTypes.generated.h"

UENUM(BlueprintType)
enum class EXRDeviceScreenOrientation : uint8
{
	Unknown             UMETA(DisplayName = "未知"),
	Portrait            UMETA(DisplayName = "竖屏"),
	PortraitUpsideDown  UMETA(DisplayName = "竖屏倒置"),
	LandscapeLeft       UMETA(DisplayName = "横屏左"),
	LandscapeRight      UMETA(DisplayName = "横屏右"),
};

UENUM(BlueprintType)
enum class EXRDeviceNetworkType : uint8
{
	Unknown       UMETA(DisplayName = "未知"),
	None          UMETA(DisplayName = "无网络"),
	AirplaneMode  UMETA(DisplayName = "飞行模式"),
	Cellular      UMETA(DisplayName = "蜂窝网络"),
	WiFi          UMETA(DisplayName = "WiFi"),
	WiMax         UMETA(DisplayName = "WiMax"),
};
