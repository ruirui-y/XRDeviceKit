// 接口契约测试：在编辑器（Win64）里跑，验证 56 个节点的注册数量和"非 Android 默认值"。
// 它证明不了真机取值对不对——那段代码在 #if PLATFORM_ANDROID 里，Win64 根本不编译。
//
// 跑法：
//   UnrealEditor-Cmd.exe XRDeviceKitDemo.uproject -ExecCmds="Automation RunTests XRDeviceKit;Quit" -unattended -nop4 -log

#include "Misc/AutomationTest.h"
#include "XRDeviceKitLibrary.h"
#include "XRDeviceKitEventSubsystem.h"
#include "XRDeviceKitTypes.h"
#include "UObject/UnrealType.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FXRDeviceKitContractTest, "XRDeviceKit.ApiContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FXRDeviceKitContractTest::RunTest(const FString& Parameters)
{
	using L = UXRDeviceKitLibrary;

	// ---- 结构检查：节点不能莫名变少 ----
	UClass* LibraryClass = L::StaticClass();
	int32 CallableCount = 0;
	for (TFieldIterator<UFunction> It(LibraryClass); It; ++It)
	{
		if (It->HasAnyFunctionFlags(FUNC_BlueprintCallable))
		{
			++CallableCount;
		}
	}
	TestTrue(FString::Printf(TEXT("蓝图节点数量应该 >= 67，实际 %d"), CallableCount), CallableCount >= 67);

	// ---- 设备 / 电源 ----
	TestEqual(TEXT("电量在非 Android 平台返回 -1"), L::GetBatteryLevel(), -1);
	TestFalse(TEXT("非 Android 不在充电"), L::IsCharging());
	TestFalse(TEXT("非 Android 不是省电模式"), L::IsLowPowerMode());
	TestTrue(TEXT("电池温度返回 -1"), FMath::IsNearlyEqual(L::GetBatteryTemperature(), -1.0f));
	TestTrue(TEXT("CPU 温度返回 -1"), FMath::IsNearlyEqual(L::GetCPUTemperature(), -1.0f));
	// 设备 ID 在 Windows 上可能是空串（实测这台机器就是），不做非空断言，只验证调用不崩
	L::GetDeviceId();
	TestFalse(TEXT("系统版本串不应该为空"), L::GetOSVersionString().IsEmpty());
	TestEqual(TEXT("安卓主版本号 -1"), L::GetAndroidMajorVersion(), -1);
	TestTrue(TEXT("CPU 型号非 Android 为空"), L::GetCPUBrand().IsEmpty());
	TestTrue(TEXT("核心数应该 > 0"), L::GetCoreCount() > 0);
	TestFalse(TEXT("默认语言不应该为空"), L::GetDefaultLocale().IsEmpty());
	TestTrue(TEXT("C 盘剩余空间应该 >= 0"), L::GetDiskFreeMegabytes(TEXT("C:/")) >= 0);
	TestTrue(TEXT("C 盘总空间应该 > 0"), L::GetDiskTotalMegabytes(TEXT("C:/")) > 0);

	// ---- 设备构建信息 ----
	TestTrue(TEXT("厂商非 Android 为空"), L::GetManufacturer().IsEmpty());
	TestTrue(TEXT("型号非 Android 为空"), L::GetModel().IsEmpty());
	TestTrue(TEXT("设备代号非 Android 为空"), L::GetDeviceName().IsEmpty());
	TestTrue(TEXT("ABI 非 Android 为空"), L::GetSupportedABIs().IsEmpty());

	// ---- 显示 ----
	TestTrue(TEXT("非 Android 屏幕当成亮着"), L::IsScreenOn());
	TestTrue(TEXT("非 Android 亮度是 1.0"), FMath::IsNearlyEqual(L::GetBrightness(), 1.0f));
	TestFalse(TEXT("非 Android 设亮度返回 false"), L::SetBrightness(0.5f));
	TestTrue(TEXT("非 Android 屏幕方向 Unknown"), L::GetScreenOrientation() == EXRDeviceScreenOrientation::Unknown);
	TestFalse(TEXT("非 Android 设方向返回 false"), L::SetScreenOrientation(EXRDeviceScreenOrientation::Portrait));
	TestEqual(TEXT("屏幕宽 0"), L::GetScreenWidth(), 0);
	TestEqual(TEXT("屏幕高 0"), L::GetScreenHeight(), 0);
	TestEqual(TEXT("DPI 0"), L::GetScreenDpi(), 0);
	TestTrue(TEXT("密度 1.0"), FMath::IsNearlyEqual(L::GetScreenDensity(), 1.0f));
	TestTrue(TEXT("刷新率 0"), FMath::IsNearlyEqual(L::GetScreenRefreshRate(), 0.0f));
	TestEqual(TEXT("系统亮度 -1"), L::GetSystemBrightness(), -1);
	TestFalse(TEXT("非 Android 不是深色模式"), L::IsNightMode());

	// ---- 网络 ----
	TestTrue(TEXT("非 Android 网络类型 Unknown"), L::GetNetworkType() == EXRDeviceNetworkType::Unknown);
	TestFalse(TEXT("非 Android 无 WiFi 判断"), L::HasWiFiConnection());

	// ---- 音频 ----
	TestTrue(TEXT("音量百分比 >= -1（-1 表示拿不到）"), L::GetDeviceVolume() >= -1);
	TestFalse(TEXT("非 Android 设音量返回 false"), L::SetVolume(50));
	TestFalse(TEXT("非 Android 无耳机判断"), L::IsHeadphonesPluggedIn());

	// ---- 存储 ----
	TestEqual(TEXT("内部剩余 -1"), L::GetInternalFreeMegabytes(), static_cast<int64>(-1));
	TestEqual(TEXT("内部总量 -1"), L::GetInternalTotalMegabytes(), static_cast<int64>(-1));
	TestEqual(TEXT("外部剩余 -1"), L::GetExternalFreeMegabytes(), static_cast<int64>(-1));
	TestEqual(TEXT("外部总量 -1"), L::GetExternalTotalMegabytes(), static_cast<int64>(-1));

	// ---- 应用 ----
	TestTrue(TEXT("包名非 Android 为空"), L::GetPackageName().IsEmpty());
	TestTrue(TEXT("版本名非 Android 为空"), L::GetVersionName().IsEmpty());
	TestEqual(TEXT("版本号 -1"), L::GetVersionCode(), static_cast<int64>(-1));
	TestTrue(TEXT("应用显示名非 Android 为空"), L::GetAppLabel().IsEmpty());
	TestEqual(TEXT("安装时间 -1"), L::GetFirstInstallTimeMs(), static_cast<int64>(-1));
	TestEqual(TEXT("更新时间 -1"), L::GetLastUpdateTimeMs(), static_cast<int64>(-1));
	TestFalse(TEXT("非 Android 拉起 App 返回 false"), L::StartApp(TEXT("com.example.none")));

	// ---- 系统 ----
	TestEqual(TEXT("开机时长 -1"), L::GetUptimeSeconds(), static_cast<int64>(-1));
	TestTrue(TEXT("时区非 Android 为空"), L::GetTimeZoneId().IsEmpty());
	TestTrue(TEXT("语言标签非 Android 为空"), L::GetLanguageTag().IsEmpty());
	TestFalse(TEXT("工程版本号不应该为空"), L::GetProjectVersion().IsEmpty());

	// ---- 剪贴板往返 ----
	const FString ClipboardProbe = TEXT("XRDeviceKitContractProbe");
	L::CopyToClipboard(ClipboardProbe);
	TestEqual(TEXT("剪贴板往返一致"), L::PasteFromClipboard(), ClipboardProbe);

	// ---- 零权限查询（补充） ----
	TestTrue(TEXT("IP 非 Android 为空"), L::GetIpAddress().IsEmpty());
	TestEqual(TEXT("息屏超时 -1"), L::GetScreenOffTimeout(), -1);
	TestTrue(TEXT("语言列表非 Android 为空"), L::GetLanguageList().IsEmpty());
	TestEqual(TEXT("铃声模式 -1"), L::GetRingerMode(), -1);
	TestEqual(TEXT("电池电流 -1"), L::GetBatteryCurrentNow(), -1);
	TestEqual(TEXT("电池健康 -1"), L::GetBatteryHealth(), -1);

	// ---- 性能 ----
	TestTrue(TEXT("渲染帧率 >= 0"), L::GetRenderFPS() >= 0.0f);
	TestTrue(TEXT("帧耗时 >= 0"), L::GetFrameTimeMs() >= 0.0f);

	// ---- 资源统计 / 硬件信息 ----
	const FXRDeviceResourceSnapshot Snapshot = L::GetResourceSnapshot();
	TestTrue(TEXT("没有子系统时快照 CPU 为 0"), FMath::IsNearlyZero(Snapshot.CpuPercent));
	const FXRDeviceHardwareInfo Hardware = L::GetHardwareInfo();
	TestTrue(TEXT("硬件信息：核心数 > 0"), Hardware.CpuCoreCount > 0);
	TestTrue(TEXT("硬件信息：内存总量 > 0"), Hardware.TotalMemoryMB > 0);
	TestTrue(TEXT("硬件信息：屏幕宽非 Android 为 0"), Hardware.ScreenWidth == 0);
	L::StartResourceLogging(1.0f);
	L::StopResourceLogging();

	// ---- 事件子系统：类注册了就够（自动化测试里没有 GameInstance） ----
	TestNotNull(TEXT("事件子系统已注册"), UXRDeviceKitEventSubsystem::StaticClass());

	// ---- 振动：只要不崩就算过 ----
	L::StartHaptics(1);
	L::StopHaptics();

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
