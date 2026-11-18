package com.xrdevicekit.bridge;

import android.content.Context;

// Entry point of the Java side of XRDeviceKit.
// GameActivity calls Init() once from onCreate, then C++ calls the static methods
// below through JNI. Everything here must stay static-friendly: no Activity needed.
public final class XRBridge
{
    // Application context, set once at startup. Application-level context never leaks.
    private static Context sContext;

    private XRBridge()
    {
    }

    public static void Init(Context context)
    {
        if (context != null)
        {
            sContext = context.getApplicationContext();
        }
    }

    // Package name of the running app, e.g. com.YourCompany.XRDeviceKitDemo.
    // Returns an empty string when the bridge was never initialised.
    public static String GetPackageName()
    {
        try
        {
            if (sContext == null)
            {
                return "";
            }
            return sContext.getPackageName();
        }
        catch (Throwable t)
        {
            return "";
        }
    }
}
