// XRDeviceKit 插件的编译脚本。
// Android 分支负责两件事：登记 UPL（把 Java 注入 GameActivity），以及补 JNI 头文件所在的模块。

using System.IO;
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

		// 剪贴板在 ApplicationCore 里（HAL/PlatformApplicationMisc.h），全平台都要
		PrivateDependencyModuleNames.Add("ApplicationCore");

		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			// Android/AndroidApplication.h 在 ApplicationCore 模块里，Android/AndroidJNI.h 在 Launch 模块里
			PrivateIncludePathModuleNames.Add("Launch");

			string APLPath = Path.Combine(ModuleDirectory, "Android", "XRDeviceKit_APL.xml");
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", APLPath);
		}
	}
}
