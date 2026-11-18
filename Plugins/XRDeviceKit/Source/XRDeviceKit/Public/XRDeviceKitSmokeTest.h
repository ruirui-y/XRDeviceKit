// 真机冒烟测试：把全部节点调一遍，按固定格式打到日志里，方便 adb 抓。
// 默认不开，靠 ini 里的 [XRDeviceKit] SmokeTestOnStartup=True 打开。

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "XRDeviceKitSmokeTest.generated.h"

UCLASS()
class XRDEVICEKIT_API UXRDeviceKitSmokeTestSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 子系统初始化时就注册；真正开跑要等 3 秒，给 Java 侧初始化和场景加载留时间
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 手动跑一轮，蓝图里也能调
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Test")
	void RunSmokeTest();
};
