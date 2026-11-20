// libsvrapi.so 探针实现。
// 注意 svrDeviceInfo 是按值返回的结构体，这里必须和 svrApi.h 的字段顺序、类型完全一致，
// 否则 ABI 对不上会读到垃圾值。

#include "XRDeviceKitSVR.h"

#if PLATFORM_ANDROID
#include <dlfcn.h>
#endif

DEFINE_LOG_CATEGORY_STATIC(LogXRDeviceKitSVR, Log, All);

namespace
{
#if PLATFORM_ANDROID
// 对应 svrApi.h 的 svrViewFrustum
struct FSVRViewFrustum
{
	float left;
	float right;
	float top;
	float bottom;
	float nearPlane;
	float farPlane;
};

// 对应 svrApi.h 的 svrDeviceInfo
struct FSVRDeviceInfo
{
	int32 DisplayWidthPixels;
	int32 DisplayHeightPixels;
	float DisplayRefreshRateHz;
	int32 DisplayOrientation;
	int32 TargetEyeWidthPixels;
	int32 TargetEyeHeightPixels;
	float TargetFovXRad;
	float TargetFovYRad;
	FSVRViewFrustum LeftEyeFrustum;
	FSVRViewFrustum RightEyeFrustum;
	float TargetEyeConvergence;
	float TargetEyePitch;
	int32 DeviceOSVersion;
	int32 WarpMeshType;
};

using FnGetVersion = const char* (*)();
using FnGetStrVersion = int32 (*)(ANSICHAR*, int32);
using FnGetDeviceInfo = FSVRDeviceInfo (*)();
using FnSetPerformanceLevels = int32 (*)(int32, int32);

void* GLibrary = nullptr;
FnGetVersion GGetVersion = nullptr;
FnGetStrVersion GGetServiceVersion = nullptr;
FnGetDeviceInfo GGetDeviceInfo = nullptr;
FnSetPerformanceLevels GSetPerformanceLevels = nullptr;
bool GLoadTried = false;

// 第一次调用时加载库并解析符号
void EnsureLoaded()
{
	if (GLoadTried)
	{
		return;
	}
	GLoadTried = true;

	GLibrary = dlopen("libsvrapi.so", RTLD_NOW);
	if (GLibrary == nullptr)
	{
		UE_LOG(LogXRDeviceKitSVR, Error, TEXT("dlopen libsvrapi.so 失败: %s"), ANSI_TO_TCHAR(dlerror()));
		return;
	}

	GGetVersion = reinterpret_cast<FnGetVersion>(dlsym(GLibrary, "svrGetVersion"));
	GGetServiceVersion = reinterpret_cast<FnGetStrVersion>(dlsym(GLibrary, "svrGetVrServiceVersion"));
	GGetDeviceInfo = reinterpret_cast<FnGetDeviceInfo>(dlsym(GLibrary, "svrGetDeviceInfo"));
	GSetPerformanceLevels = reinterpret_cast<FnSetPerformanceLevels>(dlsym(GLibrary, "svrSetPerformanceLevels"));

	UE_LOG(LogXRDeviceKitSVR, Log, TEXT("libsvrapi.so 已加载，符号：ver=%d svc=%d info=%d perf=%d"),
		GGetVersion ? 1 : 0, GGetServiceVersion ? 1 : 0, GGetDeviceInfo ? 1 : 0, GSetPerformanceLevels ? 1 : 0);
}
#endif
}

XRDeviceKit::SVR::FProbeResult XRDeviceKit::SVR::Probe()
{
	FProbeResult Result;

#if PLATFORM_ANDROID
	EnsureLoaded();
	if (GLibrary == nullptr)
	{
		return Result;
	}

	// 版本号：最安全，不需要初始化
	if (GGetVersion != nullptr)
	{
		const char* Version = GGetVersion();
		Result.ClientVersion = Version ? FString(ANSI_TO_TCHAR(Version)) : FString();
	}

	if (GGetServiceVersion != nullptr)
	{
		ANSICHAR Buffer[128] = {0};
		const int32 Code = GGetServiceVersion(Buffer, sizeof(Buffer));
		Result.ServiceVersion = Code == 0 ? FString(ANSI_TO_TCHAR(Buffer)) : FString::Printf(TEXT("err=%d"), Code);
	}

	// 设备信息：视场角、单眼分辨率这些 Android 框架拿不到
	if (GGetDeviceInfo != nullptr)
	{
		const FSVRDeviceInfo Info = GGetDeviceInfo();
		Result.bDeviceInfoOk = true;
		Result.DisplayWidth = Info.DisplayWidthPixels;
		Result.DisplayHeight = Info.DisplayHeightPixels;
		Result.RefreshRateHz = Info.DisplayRefreshRateHz;
		Result.DisplayOrientation = Info.DisplayOrientation;
		Result.EyeBufferWidth = Info.TargetEyeWidthPixels;
		Result.EyeBufferHeight = Info.TargetEyeHeightPixels;
		Result.FovXRad = Info.TargetFovXRad;
		Result.FovYRad = Info.TargetFovYRad;
		Result.WarpMeshType = Info.WarpMeshType;
	}

	// 性能档位：参数用 2/2（High），返回值只记录，不改后续行为
	if (GSetPerformanceLevels != nullptr)
	{
		Result.SetPerfResult = GSetPerformanceLevels(2, 2);
	}

	UE_LOG(LogXRDeviceKitSVR, Warning,
		TEXT("SVR|client=%s service=%s info=%d display=%dx%d@%.1fHz eye=%dx%d fov=%.2f/%.2f warp=%d perf=%d"),
		*Result.ClientVersion, *Result.ServiceVersion, Result.bDeviceInfoOk ? 1 : 0,
		Result.DisplayWidth, Result.DisplayHeight, Result.RefreshRateHz,
		Result.EyeBufferWidth, Result.EyeBufferHeight, Result.FovXRad, Result.FovYRad,
		Result.WarpMeshType, Result.SetPerfResult);
#endif

	return Result;
}
