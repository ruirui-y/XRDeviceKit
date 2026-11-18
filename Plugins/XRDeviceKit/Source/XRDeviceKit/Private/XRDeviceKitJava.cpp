// Java 层桥接的实现。类只找一次就缓存，之后每次调用只查方法。
// 这里不写业务逻辑：Java 干完活，把结果原样转换回 UE 类型。

#include "XRDeviceKitJava.h"

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJavaEnv.h"
#include "Android/AndroidJNI.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogXRDeviceKitJava, Log, All);

#if PLATFORM_ANDROID
namespace
{
jclass GXRBridgeClass = nullptr;                                                     // XRBridge 的全局引用，进程内一直有效
jmethodID GGetPackageNameMethod = nullptr;                                           // GetPackageName() 的方法 ID
bool GResolved = false;                                                              // 是否已经查过一轮

// 找一次类和方法就缓存住。类找不到说明 JavaLibs 没打进 APK，直接报错，别让调用方拿到空串还不知道为什么。
void ResolveBridge(JNIEnv* Env)
{
	if (GResolved)
	{
		return;
	}
	GResolved = true;

	GXRBridgeClass = FAndroidApplication::FindJavaClassGlobalRef("com/xrdevicekit/bridge/XRBridge");
	if (GXRBridgeClass == nullptr)
	{
		UE_LOG(LogXRDeviceKitJava, Error, TEXT("找不到 Java 类 com/xrdevicekit/bridge/XRBridge，检查 JavaLibs 是否打进 APK"));
		return;
	}

	GGetPackageNameMethod = Env->GetStaticMethodID(GXRBridgeClass, "GetPackageName", "()Ljava/lang/String;");
	if (GGetPackageNameMethod == nullptr)
	{
		UE_LOG(LogXRDeviceKitJava, Error, TEXT("XRBridge 里没有 GetPackageName() 方法，检查 Java 和 C++ 是否同步"));
	}
}
} // namespace
#endif

FString XRDeviceKit::Java::GetPackageName()
{
#if PLATFORM_ANDROID
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (Env == nullptr)
	{
		return FString();
	}

	ResolveBridge(Env);
	if (GXRBridgeClass == nullptr || GGetPackageNameMethod == nullptr)
	{
		return FString();
	}

	// 静态方法：第一个参数传 jclass，不是对象实例
	jstring JavaString = static_cast<jstring>(Env->CallStaticObjectMethod(GXRBridgeClass, GGetPackageNameMethod));
	return FJavaHelper::FStringFromLocalRef(Env, JavaString);
#else
	return FString();
#endif
}
