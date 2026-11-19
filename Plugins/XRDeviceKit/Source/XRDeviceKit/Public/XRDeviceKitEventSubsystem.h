// 系统事件子系统：把 Android 的屏幕亮灭、网络变化转成蓝图能 Bind 的事件。
//
// 链路（唯一一处 Java 主动调 C++）：
//
//   Android 广播
//     └─ XRBridge 里的 BroadcastReceiver / NetworkCallback
//         └─ native 方法 OnScreenStateChanged / OnNetworkChanged
//             └─ 本文件里的 Java_com_xrdevicekit_bridge_XRBridge_OnXxx（Java 线程）
//                 └─ AsyncTask 切 GameThread
//                     └─ 蓝图多播委托 Broadcast

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "XRDeviceKitEventSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FXRScreenStateChanged, bool, bScreenOn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FXRNetworkChanged, bool, bConnected);

UCLASS()
class XRDEVICEKIT_API UXRDeviceKitEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 屏幕亮/灭。亮 = true。
	UPROPERTY(BlueprintAssignable, Category = "XRDeviceKit|Events")
	FXRScreenStateChanged OnScreenStateChanged;

	// 默认网络连通/断开。连上 = true。
	UPROPERTY(BlueprintAssignable, Category = "XRDeviceKit|Events")
	FXRNetworkChanged OnNetworkChanged;

	// 手动开/关监听；子系统初始化时自动开，一般不用调。
	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Events")
	void StartWatching();

	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Events")
	void StopWatching();

	UFUNCTION(BlueprintCallable, Category = "XRDeviceKit|Events")
	bool IsWatching() const { return bWatching; }

	// 给 JNI 回调用的入口，内部会切到 GameThread
	static void HandleJavaScreenStateChanged(bool bScreenOn);
	static void HandleJavaNetworkChanged(bool bConnected);

private:
	static TWeakObjectPtr<UXRDeviceKitEventSubsystem> CurrentInstance;

	bool bWatching = false;                                                         // 是否已经开了监听
};
