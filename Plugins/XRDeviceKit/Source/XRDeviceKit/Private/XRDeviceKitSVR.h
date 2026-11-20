// 设备自带 libsvrapi.so（高通 SVR）的探针。
// 只调不需要手柄、也不需要 BeginVr 的接口：版本号、设备信息、性能档位。
// 全部走 dlopen/dlsym，库路径用系统名，由 linker 去 /system/lib64 找。

#pragma once

#include "CoreMinimal.h"

namespace XRDeviceKit
{
namespace SVR
{
// 一轮探测的结果，字段取不到时是 0 或空
struct FProbeResult
{
	FString ClientVersion;                                                          // svrGetVersion
	FString ServiceVersion;                                                         // svrGetVrServiceVersion
	bool bDeviceInfoOk = false;                                                     // svrGetDeviceInfo 是否拿到
	int32 DisplayWidth = 0;
	int32 DisplayHeight = 0;
	float RefreshRateHz = 0.0f;
	int32 DisplayOrientation = 0;
	int32 EyeBufferWidth = 0;
	int32 EyeBufferHeight = 0;
	float FovXRad = 0.0f;
	float FovYRad = 0.0f;
	int32 WarpMeshType = -1;
	int32 SetPerfResult = -999;                                                     // svrSetPerformanceLevels 的返回码
};

// 跑一轮探测；第一次调用时加载库，之后复用
FProbeResult Probe();
}
}
