// XRDeviceKit 插件的编译脚本。
// 现在只有模块骨架；Android 平台分支、JNI 头文件路径、大鹏 .so 的链接，等 P1 再加。

using UnrealBuildTool;

public class XRDeviceKit : ModuleRules
{
	public XRDeviceKit(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
		});
	}
}
