// 系统事件子系统的实现。
// Java 回调发生在 Java 线程（Android 主线程），不能直接碰 UObject，
// 所以两个 HandleXxx 里一律先 AsyncTask 切回 GameThread 再广播。

#include "XRDeviceKitEventSubsystem.h"
#include "XRDeviceKitJava.h"
#include "Async/Async.h"

#if PLATFORM_ANDROID
#include <jni.h>
#include "Android/AndroidJNI.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogXRDeviceKitEvents, Log, All);

TWeakObjectPtr<UXRDeviceKitEventSubsystem> UXRDeviceKitEventSubsystem::CurrentInstance;

void UXRDeviceKitEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// JNI 回调是静态函数，靠这个弱指针找到当前活着的子系统
	CurrentInstance = this;
	StartWatching();
}

void UXRDeviceKitEventSubsystem::Deinitialize()
{
	StopWatching();
	CurrentInstance = nullptr;
	Super::Deinitialize();
}

void UXRDeviceKitEventSubsystem::StartWatching()
{
	if (bWatching)
	{
		return;
	}
	bWatching = true;

	XRDeviceKit::Java::StartScreenWatcher();
	XRDeviceKit::Java::StartNetworkWatcher();

	UE_LOG(LogXRDeviceKitEvents, Log, TEXT("开始监听屏幕与网络事件"));
}

void UXRDeviceKitEventSubsystem::StopWatching()
{
	if (!bWatching)
	{
		return;
	}
	bWatching = false;

	XRDeviceKit::Java::StopScreenWatcher();
	XRDeviceKit::Java::StopNetworkWatcher();

	UE_LOG(LogXRDeviceKitEvents, Log, TEXT("停止监听屏幕与网络事件"));
}

void UXRDeviceKitEventSubsystem::HandleJavaScreenStateChanged(bool bScreenOn)
{
	UE_LOG(LogXRDeviceKitEvents, Log, TEXT("收到屏幕事件: %s"), bScreenOn ? TEXT("亮") : TEXT("灭"));

	// 这里已经在 Java 线程上，只能搬数据，不能碰 UObject
	AsyncTask(ENamedThreads::GameThread, [bScreenOn]()
	{
		if (UXRDeviceKitEventSubsystem* Self = CurrentInstance.Get())
		{
			Self->OnScreenStateChanged.Broadcast(bScreenOn);
		}
	});
}

void UXRDeviceKitEventSubsystem::HandleJavaNetworkChanged(bool bConnected)
{
	UE_LOG(LogXRDeviceKitEvents, Log, TEXT("收到网络事件: %s"), bConnected ? TEXT("已连接") : TEXT("断开"));

	AsyncTask(ENamedThreads::GameThread, [bConnected]()
	{
		if (UXRDeviceKitEventSubsystem* Self = CurrentInstance.Get())
		{
			Self->OnNetworkChanged.Broadcast(bConnected);
		}
	});
}

#if PLATFORM_ANDROID
// 函数名必须和 XRBridge.java 里的 native 声明严格对应：
// Java_<包名点换下划线>_<类名>_<方法名>
extern "C" JNIEXPORT void JNICALL Java_com_xrdevicekit_bridge_XRBridge_OnScreenStateChanged(JNIEnv* Env, jclass Clazz, jboolean bIsOn)
{
	UXRDeviceKitEventSubsystem::HandleJavaScreenStateChanged(bIsOn == JNI_TRUE);
}

extern "C" JNIEXPORT void JNICALL Java_com_xrdevicekit_bridge_XRBridge_OnNetworkChanged(JNIEnv* Env, jclass Clazz, jboolean bConnected)
{
	UXRDeviceKitEventSubsystem::HandleJavaNetworkChanged(bConnected == JNI_TRUE);
}
#endif
