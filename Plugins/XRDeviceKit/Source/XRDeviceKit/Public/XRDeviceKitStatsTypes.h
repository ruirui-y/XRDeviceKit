// 资源统计和硬件信息的数据结构。
// 两个结构体都只装数据，不装逻辑；采集逻辑在 XRDeviceKitStatsSubsystem 里。

#pragma once

#include "CoreMinimal.h"
#include "XRDeviceKitStatsTypes.generated.h"

// 某一时刻的资源快照。CPU 占用是"距上次采样"的平均值，其余是瞬时值。
USTRUCT(BlueprintType)
struct FXRDeviceResourceSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Stats")
	float CpuPercent = 0.0f;                                                        // 本进程 CPU 占用，相对整机 0~100

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Stats")
	float SystemCpuPercent = 0.0f;                                                  // 整机 CPU 占用 0~100

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Stats")
	int64 MemoryUsedMB = 0;                                                         // 本进程 RSS

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Stats")
	int64 MemoryPeakMB = 0;                                                         // 本进程峰值 RSS

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Stats")
	int64 EngineMemoryUsedMB = 0;                                                   // 引擎堆已用

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Stats")
	int64 SystemAvailableMB = 0;                                                    // 整机可用内存

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Stats")
	int32 ThreadCount = 0;                                                          // 进程内线程数

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Stats")
	float FrameTimeMs = 0.0f;                                                       // 帧耗时（stat unit 平滑值）

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Stats")
	float GameThreadMs = 0.0f;                                                      // 游戏线程

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Stats")
	float RenderThreadMs = 0.0f;                                                    // 渲染线程

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Stats")
	float RhitMs = 0.0f;                                                            // RHI 线程

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Stats")
	float GpuFrameTimeMs = 0.0f;                                                    // GPU 帧时间，设备不支持时为 0

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Stats")
	float GpuUsageFraction = 0.0f;                                                  // GPU 占用 0~1，设备不支持时为 0

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Stats")
	float IoReadMB = 0.0f;                                                          // 进程累计读盘

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Stats")
	float IoWriteMB = 0.0f;                                                         // 进程累计写盘

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Stats")
	int32 ThermalStatus = -1;                                                       // 热状态 0 无 / 1 轻 / 2 中 / 3 严重 / 4 危急 / 5 关机 / 6 未知

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Stats")
	int32 BatteryCurrentMa = -1;                                                    // 电池电流，符号随厂商

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Stats")
	float BatteryTempC = -1.0f;                                                     // 电池温度
};

// 设备硬件信息。几乎不变，所以单独一个节点，不参与每秒采样。
USTRUCT(BlueprintType)
struct FXRDeviceHardwareInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Hardware")
	FString Manufacturer;                                                           // 厂商

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Hardware")
	FString Model;                                                                  // 型号

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Hardware")
	FString DeviceName;                                                             // 设备代号

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Hardware")
	FString CpuBrand;                                                               // CPU 品牌（安卓上等于型号）

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Hardware")
	int32 CpuCoreCount = 0;                                                         // 物理核心数

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Hardware")
	FString Abi;                                                                    // 支持的 ABI

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Hardware")
	FString GpuBrand;                                                               // GPU 型号

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Hardware")
	FString GpuFamily;                                                              // GPU 家族

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Hardware")
	FString GlVersion;                                                              // OpenGL ES 版本

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Hardware")
	FString VulkanVersion;                                                          // Vulkan 版本

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Hardware")
	int64 TotalMemoryMB = 0;                                                        // 整机内存

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Hardware")
	int64 TotalStorageMB = 0;                                                       // 数据分区总容量

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Hardware")
	int32 ScreenWidth = 0;                                                          // 屏幕宽

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Hardware")
	int32 ScreenHeight = 0;                                                         // 屏幕高

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Hardware")
	int32 ScreenDpi = 0;                                                            // 屏幕密度 dpi

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Hardware")
	float RefreshRate = 0.0f;                                                       // 设备支持的最高刷新率

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Hardware")
	FString AndroidVersion;                                                         // 安卓版本串

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Hardware")
	int32 AndroidSdk = -1;                                                          // 安卓 API 级别

	UPROPERTY(BlueprintReadOnly, Category = "XRDeviceKit|Hardware")
	FString PackageName;                                                            // 运行中的包名
};
