package com.xrdevicekit.bridge;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.os.StatFs;
import android.os.SystemClock;

import java.util.Locale;
import java.util.TimeZone;

// Entry point of the Java side of XRDeviceKit.
// GameActivity calls Init() once from onCreate, then C++ calls the static methods
// below through JNI. Everything here must stay static-friendly: no Activity needed.
// All methods swallow their own exceptions and return a safe default, so nothing
// ever leaks back into JNI as a pending Java exception.
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

    // ------------------------------------------------------------------
    // App info
    // ------------------------------------------------------------------

    // Package name of the running app, e.g. com.YourCompany.XRDeviceKitDemo.
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

    // Human readable version, e.g. "1.0".
    public static String GetVersionName()
    {
        try
        {
            PackageInfo info = ReadPackageInfo(sContext.getPackageName());
            return info == null ? "" : String.valueOf(info.versionName);
        }
        catch (Throwable t)
        {
            return "";
        }
    }

    // Monotonic build number.
    public static long GetVersionCode()
    {
        try
        {
            PackageInfo info = ReadPackageInfo(sContext.getPackageName());
            // longVersionCode exists since API 28, minSdk here is 29
            return info == null ? -1L : info.getLongVersionCode();
        }
        catch (Throwable t)
        {
            return -1L;
        }
    }

    // True when a package is installed and visible to us.
    // Note: Android 11+ hides other packages unless they are declared in the
    // manifest <queries> block. A false result may simply mean "not visible".
    public static boolean IsAppInstalled(String packageName)
    {
        try
        {
            return ReadPackageInfo(packageName) != null;
        }
        catch (Throwable t)
        {
            return false;
        }
    }

    // ------------------------------------------------------------------
    // Screen
    // ------------------------------------------------------------------

    public static int GetScreenWidth()
    {
        try
        {
            return sContext.getResources().getDisplayMetrics().widthPixels;
        }
        catch (Throwable t)
        {
            return 0;
        }
    }

    public static int GetScreenHeight()
    {
        try
        {
            return sContext.getResources().getDisplayMetrics().heightPixels;
        }
        catch (Throwable t)
        {
            return 0;
        }
    }

    // Density in dpi (e.g. 320 for a 2x screen).
    public static int GetScreenDpi()
    {
        try
        {
            return sContext.getResources().getDisplayMetrics().densityDpi;
        }
        catch (Throwable t)
        {
            return 0;
        }
    }

    // Scale factor relative to a 160 dpi baseline (e.g. 2.0 for a 2x screen).
    public static float GetScreenDensity()
    {
        try
        {
            return sContext.getResources().getDisplayMetrics().density;
        }
        catch (Throwable t)
        {
            return 1.0f;
        }
    }

    // Max refresh rate the device can do (the physical panel supports several modes:
    // 90 and 72 on the P1Max). The app itself renders on a virtual 2D display, so its own
    // active mode is not the panel capability - we take the maximum over all displays' modes.
    public static float GetScreenRefreshRate()
    {
        try
        {
            android.hardware.display.DisplayManager dm =
                (android.hardware.display.DisplayManager) sContext.getSystemService(Context.DISPLAY_SERVICE);
            float best = 0.0f;
            if (dm != null)
            {
                for (android.view.Display display : dm.getDisplays())
                {
                    if (display == null)
                    {
                        continue;
                    }
                    for (android.view.Display.Mode mode : display.getSupportedModes())
                    {
                        if (mode != null && mode.getRefreshRate() > best)
                        {
                            best = mode.getRefreshRate();
                        }
                    }
                    // 诊断用：打包后 adb logcat -s XRBridge 能看到这个 App 实际能枚举到几块屏
                    android.util.Log.i("XRBridge", "display " + display.getName()
                        + " " + display.getWidth() + "x" + display.getHeight()
                        + " active=" + (display.getMode() == null ? 0.0f : display.getMode().getRefreshRate()));
                }
            }
            if (best > 0.0f)
            {
                return best;
            }

            // fallback: the display the window manager hands out
            android.view.WindowManager wm =
                (android.view.WindowManager) sContext.getSystemService(Context.WINDOW_SERVICE);
            if (wm == null)
            {
                return 0.0f;
            }
            return wm.getDefaultDisplay().getRefreshRate();
        }
        catch (Throwable t)
        {
            return 0.0f;
        }
    }

    // ------------------------------------------------------------------
    // Storage (values in megabytes)
    // ------------------------------------------------------------------

    public static long GetInternalFreeMegabytes()
    {
        return FreeMegabytesOf(sContext.getFilesDir().getAbsolutePath());
    }

    public static long GetInternalTotalMegabytes()
    {
        return TotalMegabytesOf(sContext.getFilesDir().getAbsolutePath());
    }

    public static long GetExternalFreeMegabytes()
    {
        java.io.File dir = sContext.getExternalFilesDir(null);
        if (dir == null)
        {
            return -1L;
        }
        return FreeMegabytesOf(dir.getAbsolutePath());
    }

    public static long GetExternalTotalMegabytes()
    {
        java.io.File dir = sContext.getExternalFilesDir(null);
        if (dir == null)
        {
            return -1L;
        }
        return TotalMegabytesOf(dir.getAbsolutePath());
    }

    // ------------------------------------------------------------------
    // System
    // ------------------------------------------------------------------

    // Seconds since boot, including deep sleep.
    public static long GetUptimeSeconds()
    {
        try
        {
            return SystemClock.elapsedRealtime() / 1000L;
        }
        catch (Throwable t)
        {
            return -1L;
        }
    }

    // Time zone id, e.g. "Asia/Shanghai".
    public static String GetTimeZoneId()
    {
        try
        {
            return TimeZone.getDefault().getID();
        }
        catch (Throwable t)
        {
            return "";
        }
    }

    // Language tag, e.g. "zh-CN".
    public static String GetLanguageTag()
    {
        try
        {
            return Locale.getDefault().toLanguageTag();
        }
        catch (Throwable t)
        {
            return "";
        }
    }

    // ------------------------------------------------------------------
    // Device build info
    // ------------------------------------------------------------------

    // Manufacturer, e.g. "DPVR" or "PICO".
    public static String GetManufacturer()
    {
        try
        {
            return android.os.Build.MANUFACTURER;
        }
        catch (Throwable t)
        {
            return "";
        }
    }

    // Marketing model name, e.g. "PICO 4".
    public static String GetModel()
    {
        try
        {
            return android.os.Build.MODEL;
        }
        catch (Throwable t)
        {
            return "";
        }
    }

    // Industrial device name (ro.product.device), often the codename.
    public static String GetDeviceName()
    {
        try
        {
            return android.os.Build.DEVICE;
        }
        catch (Throwable t)
        {
            return "";
        }
    }

    // Supported ABIs, comma separated, e.g. "arm64-v8a,armeabi-v7a".
    public static String GetSupportedABIs()
    {
        try
        {
            return android.text.TextUtils.join(",", android.os.Build.SUPPORTED_ABIS);
        }
        catch (Throwable t)
        {
            return "";
        }
    }

    // ------------------------------------------------------------------
    // App info (extra)
    // ------------------------------------------------------------------

    // First install time in milliseconds since epoch, or -1.
    public static long GetFirstInstallTimeMs()
    {
        try
        {
            PackageInfo info = ReadPackageInfo(sContext.getPackageName());
            return info == null ? -1L : info.firstInstallTime;
        }
        catch (Throwable t)
        {
            return -1L;
        }
    }

    // Last update time in milliseconds since epoch, or -1.
    public static long GetLastUpdateTimeMs()
    {
        try
        {
            PackageInfo info = ReadPackageInfo(sContext.getPackageName());
            return info == null ? -1L : info.lastUpdateTime;
        }
        catch (Throwable t)
        {
            return -1L;
        }
    }

    // Display name shown under the launcher icon, e.g. "XRDeviceKitDemo".
    public static String GetAppLabel()
    {
        try
        {
            android.content.pm.ApplicationInfo info = sContext.getApplicationInfo();
            return String.valueOf(sContext.getPackageManager().getApplicationLabel(info));
        }
        catch (Throwable t)
        {
            return "";
        }
    }

    // Launch another app by package name.
    // Android 11+ package visibility: a package that is not declared in the manifest
    // <queries> block is invisible here, so this returns false even when it is installed.
    public static boolean StartApp(String packageName)
    {
        try
        {
            android.content.Intent intent =
                sContext.getPackageManager().getLaunchIntentForPackage(packageName);
            if (intent == null)
            {
                return false;
            }
            intent.addFlags(android.content.Intent.FLAG_ACTIVITY_NEW_TASK
                | android.content.Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
            sContext.startActivity(intent);
            return true;
        }
        catch (Throwable t)
        {
            return false;
        }
    }

    // ------------------------------------------------------------------
    // Audio (extra)
    // ------------------------------------------------------------------

    // Set media volume by percentage (0..100).
    // The engine reports volume as 0..100, so this takes the same scale and converts
    // to the platform's own index steps internally.
    public static boolean SetVolume(int percent)
    {
        try
        {
            android.media.AudioManager am =
                (android.media.AudioManager) sContext.getSystemService(Context.AUDIO_SERVICE);
            if (am == null)
            {
                return false;
            }
            int maxIndex = am.getStreamMaxVolume(android.media.AudioManager.STREAM_MUSIC);
            int index = Math.round(percent / 100.0f * maxIndex);
            // clamp: setStreamVolume with an out of range index throws on some devices
            index = Math.max(0, Math.min(maxIndex, index));
            // flags 0: no UI shown, no sound played
            am.setStreamVolume(android.media.AudioManager.STREAM_MUSIC, index, 0);
            return true;
        }
        catch (Throwable t)
        {
            return false;
        }
    }

    // ------------------------------------------------------------------
    // Display (extra)
    // ------------------------------------------------------------------

    // System screen brightness in the platform's own scale (usually 0..255), or -1.
    // Reading needs no permission; writing needs WRITE_SETTINGS, so there is no setter here.
    public static int GetSystemBrightness()
    {
        try
        {
            return android.provider.Settings.System.getInt(
                sContext.getContentResolver(),
                android.provider.Settings.System.SCREEN_BRIGHTNESS,
                -1);
        }
        catch (Throwable t)
        {
            return -1;
        }
    }

    // True when the system is in dark mode.
    public static boolean IsNightMode()
    {
        try
        {
            int mode = sContext.getResources().getConfiguration().uiMode
                & android.content.res.Configuration.UI_MODE_NIGHT_MASK;
            return mode == android.content.res.Configuration.UI_MODE_NIGHT_YES;
        }
        catch (Throwable t)
        {
            return false;
        }
    }

    // ------------------------------------------------------------------
    // internal helpers
    // ------------------------------------------------------------------

    // PackageInfo for one package, or null when it is missing / not visible.
    private static PackageInfo ReadPackageInfo(String packageName)
    {
        try
        {
            if (sContext == null || packageName == null || packageName.isEmpty())
            {
                return null;
            }
            return sContext.getPackageManager().getPackageInfo(packageName, 0);
        }
        catch (Throwable t)
        {
            return null;
        }
    }

    private static long FreeMegabytesOf(String path)
    {
        try
        {
            StatFs stat = new StatFs(path);
            return stat.getAvailableBytes() / (1024L * 1024L);
        }
        catch (Throwable t)
        {
            return -1L;
        }
    }

    private static long TotalMegabytesOf(String path)
    {
        try
        {
            StatFs stat = new StatFs(path);
            return stat.getTotalBytes() / (1024L * 1024L);
        }
        catch (Throwable t)
        {
            return -1L;
        }
    }
}
