// Java 层桥接的实现。类只找一次就缓存，方法 ID 按"名字+签名"缓存。
// 这里不写业务逻辑：Java 干完活，把结果原样转换回 UE 类型。
// 所有 Java 方法都自己吞掉异常并返回默认值，所以这里不需要检查 Java 异常。

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
bool GResolved = false;                                                              // 是否已经查过类
TMap<FString, jmethodID> GMethodCache;                                               // 方法 ID 缓存，键是"名字+签名"

// 找一次类就缓存。找不到说明 JavaLibs 没打进 APK，直接报错，别让调用方拿到空串还不知道为什么。
bool EnsureBridgeClass(JNIEnv* Env)
{
	if (GResolved)
	{
		return GXRBridgeClass != nullptr;
	}
	GResolved = true;

	GXRBridgeClass = FAndroidApplication::FindJavaClassGlobalRef("com/xrdevicekit/bridge/XRBridge");
	if (GXRBridgeClass == nullptr)
	{
		UE_LOG(LogXRDeviceKitJava, Error, TEXT("找不到 Java 类 com/xrdevicekit/bridge/XRBridge，检查 JavaLibs 是否打进 APK"));
		return false;
	}
	return true;
}

// 按名字和签名找方法，找到过就复用缓存。
jmethodID FindBridgeMethod(JNIEnv* Env, const ANSICHAR* Name, const ANSICHAR* Signature)
{
	if (!EnsureBridgeClass(Env))
	{
		return nullptr;
	}

	const FString Key = FString::Printf(TEXT("%s%s"), ANSI_TO_TCHAR(Name), ANSI_TO_TCHAR(Signature));
	if (const jmethodID* Cached = GMethodCache.Find(Key))
	{
		return *Cached;
	}

	jmethodID Method = Env->GetStaticMethodID(GXRBridgeClass, Name, Signature);
	if (Method == nullptr)
	{
		UE_LOG(LogXRDeviceKitJava, Error, TEXT("XRBridge 里找不到 %s%s，检查 Java 和 C++ 是否同步"), ANSI_TO_TCHAR(Name), ANSI_TO_TCHAR(Signature));
		return nullptr;
	}

	GMethodCache.Add(Key, Method);
	return Method;
}

int32 CallInt(const ANSICHAR* Name)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (Env == nullptr)
	{
		return 0;
	}
	jmethodID Method = FindBridgeMethod(Env, Name, "()I");
	// 方法签名不匹配时直接返回默认值，不让 JNI 拿着空 ID 往下走
	return Method ? static_cast<int32>(Env->CallStaticIntMethod(GXRBridgeClass, Method)) : 0;
}

int64 CallLong(const ANSICHAR* Name)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (Env == nullptr)
	{
		return -1;
	}
	jmethodID Method = FindBridgeMethod(Env, Name, "()J");
	return Method ? static_cast<int64>(Env->CallStaticLongMethod(GXRBridgeClass, Method)) : -1;
}

float CallFloat(const ANSICHAR* Name)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (Env == nullptr)
	{
		return 0.0f;
	}
	jmethodID Method = FindBridgeMethod(Env, Name, "()F");
	return Method ? static_cast<float>(Env->CallStaticFloatMethod(GXRBridgeClass, Method)) : 0.0f;
}

bool CallBool(const ANSICHAR* Name)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (Env == nullptr)
	{
		return false;
	}
	jmethodID Method = FindBridgeMethod(Env, Name, "()Z");
	return Method != nullptr && Env->CallStaticBooleanMethod(GXRBridgeClass, Method) == JNI_TRUE;
}

FString CallString(const ANSICHAR* Name)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (Env == nullptr)
	{
		return FString();
	}
	jmethodID Method = FindBridgeMethod(Env, Name, "()Ljava/lang/String;");
	if (Method == nullptr)
	{
		return FString();
	}
	// FStringFromLocalRef 会顺手把局部引用删掉，不用自己 DeleteLocalRef
	jstring JavaString = static_cast<jstring>(Env->CallStaticObjectMethod(GXRBridgeClass, Method));
	return FJavaHelper::FStringFromLocalRef(Env, JavaString);
}

bool CallBoolWithString(const ANSICHAR* Name, const FString& Argument)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (Env == nullptr)
	{
		return false;
	}
	jmethodID Method = FindBridgeMethod(Env, Name, "(Ljava/lang/String;)Z");
	if (Method == nullptr)
	{
		return false;
	}

	jstring JavaArgument = Env->NewStringUTF(TCHAR_TO_UTF8(*Argument));
	const jboolean Result = Env->CallStaticBooleanMethod(GXRBridgeClass, Method, JavaArgument);
	Env->DeleteLocalRef(JavaArgument);
	return Result == JNI_TRUE;
}

void CallVoid(const ANSICHAR* Name)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (Env == nullptr)
	{
		return;
	}
	jmethodID Method = FindBridgeMethod(Env, Name, "()V");
	if (Method != nullptr)
	{
		Env->CallStaticVoidMethod(GXRBridgeClass, Method);
	}
}

bool CallBoolWithInt(const ANSICHAR* Name, int32 Value)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (Env == nullptr)
	{
		return false;
	}
	jmethodID Method = FindBridgeMethod(Env, Name, "(I)Z");
	if (Method == nullptr)
	{
		return false;
	}
	return Env->CallStaticBooleanMethod(GXRBridgeClass, Method, static_cast<jint>(Value)) == JNI_TRUE;
}
} // namespace
#endif

// ==================== 应用信息 ====================

FString XRDeviceKit::Java::GetPackageName()
{
#if PLATFORM_ANDROID
	return CallString("GetPackageName");
#else
	return FString();
#endif
}

FString XRDeviceKit::Java::GetVersionName()
{
#if PLATFORM_ANDROID
	return CallString("GetVersionName");
#else
	return FString();
#endif
}

int64 XRDeviceKit::Java::GetVersionCode()
{
#if PLATFORM_ANDROID
	return CallLong("GetVersionCode");
#else
	return -1;
#endif
}

bool XRDeviceKit::Java::IsAppInstalled(const FString& PackageName)
{
#if PLATFORM_ANDROID
	return CallBoolWithString("IsAppInstalled", PackageName);
#else
	return false;
#endif
}

// ==================== 屏幕 ====================

int32 XRDeviceKit::Java::GetScreenWidth()
{
#if PLATFORM_ANDROID
	return CallInt("GetScreenWidth");
#else
	return 0;
#endif
}

int32 XRDeviceKit::Java::GetScreenHeight()
{
#if PLATFORM_ANDROID
	return CallInt("GetScreenHeight");
#else
	return 0;
#endif
}

int32 XRDeviceKit::Java::GetScreenDpi()
{
#if PLATFORM_ANDROID
	return CallInt("GetScreenDpi");
#else
	return 0;
#endif
}

float XRDeviceKit::Java::GetScreenDensity()
{
#if PLATFORM_ANDROID
	return CallFloat("GetScreenDensity");
#else
	return 1.0f;
#endif
}

float XRDeviceKit::Java::GetScreenRefreshRate()
{
#if PLATFORM_ANDROID
	return CallFloat("GetScreenRefreshRate");
#else
	return 0.0f;
#endif
}

// ==================== 存储 ====================

int64 XRDeviceKit::Java::GetInternalFreeMegabytes()
{
#if PLATFORM_ANDROID
	return CallLong("GetInternalFreeMegabytes");
#else
	return -1;
#endif
}

int64 XRDeviceKit::Java::GetInternalTotalMegabytes()
{
#if PLATFORM_ANDROID
	return CallLong("GetInternalTotalMegabytes");
#else
	return -1;
#endif
}

int64 XRDeviceKit::Java::GetExternalFreeMegabytes()
{
#if PLATFORM_ANDROID
	return CallLong("GetExternalFreeMegabytes");
#else
	return -1;
#endif
}

int64 XRDeviceKit::Java::GetExternalTotalMegabytes()
{
#if PLATFORM_ANDROID
	return CallLong("GetExternalTotalMegabytes");
#else
	return -1;
#endif
}

// ==================== 系统 ====================

int64 XRDeviceKit::Java::GetUptimeSeconds()
{
#if PLATFORM_ANDROID
	return CallLong("GetUptimeSeconds");
#else
	return -1;
#endif
}

FString XRDeviceKit::Java::GetTimeZoneId()
{
#if PLATFORM_ANDROID
	return CallString("GetTimeZoneId");
#else
	return FString();
#endif
}

FString XRDeviceKit::Java::GetLanguageTag()
{
#if PLATFORM_ANDROID
	return CallString("GetLanguageTag");
#else
	return FString();
#endif
}

// ==================== 设备构建信息 ====================

FString XRDeviceKit::Java::GetManufacturer()
{
#if PLATFORM_ANDROID
	return CallString("GetManufacturer");
#else
	return FString();
#endif
}

FString XRDeviceKit::Java::GetModel()
{
#if PLATFORM_ANDROID
	return CallString("GetModel");
#else
	return FString();
#endif
}

FString XRDeviceKit::Java::GetDeviceName()
{
#if PLATFORM_ANDROID
	return CallString("GetDeviceName");
#else
	return FString();
#endif
}

FString XRDeviceKit::Java::GetSupportedABIs()
{
#if PLATFORM_ANDROID
	return CallString("GetSupportedABIs");
#else
	return FString();
#endif
}

// ==================== 应用信息（补充） ====================

int64 XRDeviceKit::Java::GetFirstInstallTimeMs()
{
#if PLATFORM_ANDROID
	return CallLong("GetFirstInstallTimeMs");
#else
	return -1;
#endif
}

int64 XRDeviceKit::Java::GetLastUpdateTimeMs()
{
#if PLATFORM_ANDROID
	return CallLong("GetLastUpdateTimeMs");
#else
	return -1;
#endif
}

FString XRDeviceKit::Java::GetAppLabel()
{
#if PLATFORM_ANDROID
	return CallString("GetAppLabel");
#else
	return FString();
#endif
}

bool XRDeviceKit::Java::StartApp(const FString& PackageName)
{
#if PLATFORM_ANDROID
	return CallBoolWithString("StartApp", PackageName);
#else
	return false;
#endif
}

// ==================== 音频（补充） ====================

bool XRDeviceKit::Java::SetVolume(int32 Percent)
{
#if PLATFORM_ANDROID
	return CallBoolWithInt("SetVolume", Percent);
#else
	return false;
#endif
}

// ==================== 显示（补充） ====================

int32 XRDeviceKit::Java::GetSystemBrightness()
{
#if PLATFORM_ANDROID
	return CallInt("GetSystemBrightness");
#else
	return -1;
#endif
}

bool XRDeviceKit::Java::IsNightMode()
{
#if PLATFORM_ANDROID
	return CallBool("IsNightMode");
#else
	return false;
#endif
}

// ==================== 事件监听开关 ====================

void XRDeviceKit::Java::StartScreenWatcher()
{
#if PLATFORM_ANDROID
	CallVoid("StartScreenWatcher");
#endif
}

void XRDeviceKit::Java::StopScreenWatcher()
{
#if PLATFORM_ANDROID
	CallVoid("StopScreenWatcher");
#endif
}

void XRDeviceKit::Java::StartNetworkWatcher()
{
#if PLATFORM_ANDROID
	CallVoid("StartNetworkWatcher");
#endif
}

void XRDeviceKit::Java::StopNetworkWatcher()
{
#if PLATFORM_ANDROID
	CallVoid("StopNetworkWatcher");
#endif
}

// ==================== 零权限查询（补充） ====================

FString XRDeviceKit::Java::GetIpAddress()
{
#if PLATFORM_ANDROID
	return CallString("GetIpAddress");
#else
	return FString();
#endif
}

int32 XRDeviceKit::Java::GetScreenOffTimeout()
{
#if PLATFORM_ANDROID
	return CallInt("GetScreenOffTimeout");
#else
	return -1;
#endif
}

FString XRDeviceKit::Java::GetLanguageList()
{
#if PLATFORM_ANDROID
	return CallString("GetLanguageList");
#else
	return FString();
#endif
}

int32 XRDeviceKit::Java::GetRingerMode()
{
#if PLATFORM_ANDROID
	return CallInt("GetRingerMode");
#else
	return -1;
#endif
}

int32 XRDeviceKit::Java::GetBatteryCurrentNow()
{
#if PLATFORM_ANDROID
	return CallInt("GetBatteryCurrentNow");
#else
	return -1;
#endif
}

int32 XRDeviceKit::Java::GetBatteryHealth()
{
#if PLATFORM_ANDROID
	return CallInt("GetBatteryHealth");
#else
	return -1;
#endif
}
