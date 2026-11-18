// JNI 层的实现。方法名和签名必须和 Android/XRDeviceKit_APL.xml 里注入的 Java 完全一致：
// 改了一边没改另一边，真机上会直接断言崩溃（Failed to find GXxxMethod）。

#include "XRDeviceKitJni.h"

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogXRDeviceKitJni, Log, All);

#if PLATFORM_ANDROID
namespace
{
// 三个 Java 方法 ID，第一次调用时查出来存在这里；nullptr 表示还没查
jmethodID GIsScreenOnMethod = nullptr;
jmethodID GGetBrightnessMethod = nullptr;
jmethodID GSetBrightnessPercentMethod = nullptr;
bool GMethodsResolved = false;                                                       // 是否已经查过一轮

// 把 GameActivity 上注入的三个方法 ID 查出来。
// bIsOptional 传 false：查不到直接断言，第一时间暴露"XML 没生效 / 方法名写错"。
void ResolveMethods(JNIEnv* Env)
{
	if (GMethodsResolved)
	{
		return;
	}
	GMethodsResolved = true;

	GIsScreenOnMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "XRDeviceKit_IsScreenOn", "()Z", false);
	GGetBrightnessMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "XRDeviceKit_GetBrightness", "()F", false);
	GSetBrightnessPercentMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "XRDeviceKit_SetBrightnessPercent", "(I)Z", false);

	UE_LOG(LogXRDeviceKitJni, Log, TEXT("JNI 方法已缓存: IsScreenOn=%p GetBrightness=%p SetBrightness=%p"),
		(void*)GIsScreenOnMethod, (void*)GGetBrightnessMethod, (void*)GSetBrightnessPercentMethod);
}
} // namespace
#endif

bool XRDeviceKit::Jni::IsScreenOn()
{
#if PLATFORM_ANDROID
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (Env == nullptr)
	{
		UE_LOG(LogXRDeviceKitJni, Warning, TEXT("IsScreenOn: 拿不到 JNIEnv，Activity 可能还没挂上"));
		return false;
	}
	ResolveMethods(Env);
	return FJavaWrapper::CallBooleanMethod(Env, FJavaWrapper::GameActivityThis, GIsScreenOnMethod);
#else
	return true;
#endif
}

float XRDeviceKit::Jni::GetBrightness()
{
#if PLATFORM_ANDROID
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (Env == nullptr)
	{
		UE_LOG(LogXRDeviceKitJni, Warning, TEXT("GetBrightness: 拿不到 JNIEnv"));
		return -1.0f;
	}
	ResolveMethods(Env);
	return FJavaWrapper::CallFloatMethod(Env, FJavaWrapper::GameActivityThis, GGetBrightnessMethod);
#else
	return 1.0f;
#endif
}

bool XRDeviceKit::Jni::SetBrightness(float Brightness)
{
#if PLATFORM_ANDROID
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (Env == nullptr)
	{
		UE_LOG(LogXRDeviceKitJni, Warning, TEXT("SetBrightness: 拿不到 JNIEnv"));
		return false;
	}
	ResolveMethods(Env);

	// 用 0~100 的整数过 JNI：JNI 的可变参数接口会把 float 默认提升成 double，
	// 传 float 在部分 ABI 上会读错寄存器，走整数最省心
	const int32 Percent = FMath::RoundToInt(FMath::Clamp(Brightness, 0.0f, 1.0f) * 100.0f);
	return FJavaWrapper::CallBooleanMethod(Env, FJavaWrapper::GameActivityThis, GSetBrightnessPercentMethod, Percent);
#else
	UE_LOG(LogXRDeviceKitJni, Verbose, TEXT("SetBrightness 在非安卓平台是空实现"));
	return false;
#endif
}
