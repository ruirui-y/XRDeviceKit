// Java 层桥接：调用 JavaLibs/xrbridge 里的静态方法。
// 这一层只认 com/xrdevicekit/bridge/XRBridge 这一个类；方法名和签名必须和 XRBridge.java 一致，
// 改了一边没改另一边，真机上会直接报"找不到方法"。

#pragma once

#include "CoreMinimal.h"

namespace XRDeviceKit
{
namespace Java
{
// 运行中的包名，例如 com.YourCompany.XRDeviceKitDemo；非 Android 或失败返回空串
FString GetPackageName();
}
}
