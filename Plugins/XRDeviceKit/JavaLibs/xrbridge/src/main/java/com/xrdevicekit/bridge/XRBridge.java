package com.xrdevicekit.bridge;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.os.StatFs;
import android.os.SystemClock;

import java.util.Locale;
import java.util.TimeZone;

// XRDeviceKit 的 Java 侧入口。
// GameActivity 在 onCreate 里调一次 Init()，之后 C++ 通过 JNI 调下面这些静态方法。
// 这里的方法必须能脱离 Activity 使用，只靠 Application Context。
// 每个方法自己吞异常并返回安全默认值，
// 不让异常漏回 JNI 变成待处理异常（那样下一次 JNI 调用会崩）。
public final class XRBridge
{
    // Application Context，启动时设一次；Application 级不会泄漏。
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
    // 应用信息
    // ------------------------------------------------------------------

    // 运行中的包名，例如 com.YourCompany.XRDeviceKitDemo。
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

    // 版本名，例如 "1.0"。
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

    // 构建号，只增不减。
    public static long GetVersionCode()
    {
        try
        {
            PackageInfo info = ReadPackageInfo(sContext.getPackageName());
            // longVersionCode 从 API 28 才有，本模块 minSdk 是 29
            return info == null ? -1L : info.getLongVersionCode();
        }
        catch (Throwable t)
        {
            return -1L;
        }
    }

    // 包已安装且对我们可见时返回 true。
    // 注意：Android 11+ 会隐藏未在清单 <queries> 里声明的包，
    // 返回 false 可能只是"看不见"，不代表没装。
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
    // 屏幕
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

    // 屏幕密度 dpi，例如 2 倍屏是 320。
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

    // 相对 160dpi 的倍率，例如 2 倍屏是 2.0。
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

    // 设备能跑的最高刷新率。物理屏支持多个模式（P1Max 上是 90 和 72），
    // 而 App 自己画在 2D 虚拟屏上，
    // 它的当前模式不代表面板能力，所以取所有屏幕模式里的最大值。
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

            // 兜底：退回窗口管理器给的那块屏
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
    // 存储（单位 MB）
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
    // 系统
    // ------------------------------------------------------------------

    // 开机时长（秒），含深睡眠时间。
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

    // 时区 ID，例如 "Asia/Shanghai"。
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

    // 语言标签，例如 "zh-CN"。
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
    // 设备构建信息
    // ------------------------------------------------------------------

    // 厂商，例如 "DPVR"。
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

    // 型号，例如 "P1Max"。
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

    // 设备代号（ro.product.device），通常是内部代号。
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

    // 支持的 ABI，逗号分隔，例如 "arm64-v8a,armeabi-v7a"。
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
    // 应用信息（补充）
    // ------------------------------------------------------------------

    // 首次安装时间（毫秒时间戳），拿不到返回 -1。
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

    // 最后更新时间（毫秒时间戳），拿不到返回 -1。
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

    // 桌面图标下面显示的名字，例如 "XRDeviceKitDemo"。
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

    // 按包名拉起另一个 App。
    // Android 11+ 包可见性：没在清单 <queries> 里声明的包，
    // 即使装了也返回 false。
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
    // 音频（补充）
    // ------------------------------------------------------------------

    // 按百分比设置媒体音量（0~100）。
    // 引擎的音量是 0~100，这里用同样的刻度，
    // 内部再换算成设备自己的档位。
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
            // 夹紧：部分设备上越界的档位会让 setStreamVolume 抛异常
            index = Math.max(0, Math.min(maxIndex, index));
            // flags 传 0：不弹音量条、不放提示音
            am.setStreamVolume(android.media.AudioManager.STREAM_MUSIC, index, 0);
            return true;
        }
        catch (Throwable t)
        {
            return false;
        }
    }

    // ------------------------------------------------------------------
    // 显示（补充）
    // ------------------------------------------------------------------

    // 系统亮度，用平台自己的刻度（一般是 0~255），拿不到返回 -1。
    // 读不需要权限；写需要 WRITE_SETTINGS，所以这里不提供 setter。
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

    // 系统处于深色模式时返回 true。
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
    // 零权限查询（补充）
    // ------------------------------------------------------------------

    // 第一个非回环的 IPv4 地址，例如 "192.168.1.23"。
    // 读网卡不需要权限；拿 SSID 才需要，所以这个不是 WiFi 名称。
    public static String GetIpAddress()
    {
        try
        {
            java.util.Enumeration<java.net.NetworkInterface> interfaces =
                java.net.NetworkInterface.getNetworkInterfaces();
            while (interfaces != null && interfaces.hasMoreElements())
            {
                java.net.NetworkInterface nif = interfaces.nextElement();
                if (nif == null || !nif.isUp() || nif.isLoopback())
                {
                    continue;
                }
                java.util.Enumeration<java.net.InetAddress> addresses = nif.getInetAddresses();
                while (addresses.hasMoreElements())
                {
                    java.net.InetAddress address = addresses.nextElement();
                    if (address != null
                        && !address.isLoopbackAddress()
                        && address instanceof java.net.Inet4Address)
                    {
                        return address.getHostAddress();
                    }
                }
            }
            return "";
        }
        catch (Throwable t)
        {
            return "";
        }
    }

    // 息屏超时（毫秒），拿不到返回 -1。
    public static int GetScreenOffTimeout()
    {
        try
        {
            return android.provider.Settings.System.getInt(
                sContext.getContentResolver(),
                android.provider.Settings.System.SCREEN_OFF_TIMEOUT,
                -1);
        }
        catch (Throwable t)
        {
            return -1;
        }
    }

    // 首选语言列表，例如 "zh-CN,en-US"；API 24 以下退化成单个标签。
    public static String GetLanguageList()
    {
        try
        {
            if (android.os.Build.VERSION.SDK_INT >= 24)
            {
                return android.os.LocaleList.getDefault().toLanguageTags();
            }
            return Locale.getDefault().toLanguageTag();
        }
        catch (Throwable t)
        {
            return "";
        }
    }

    // 铃声模式：0 静音 / 1 振动 / 2 正常，-1 未知。
    public static int GetRingerMode()
    {
        try
        {
            android.media.AudioManager am =
                (android.media.AudioManager) sContext.getSystemService(Context.AUDIO_SERVICE);
            return am == null ? -1 : am.getRingerMode();
        }
        catch (Throwable t)
        {
            return -1;
        }
    }

    // 电池瞬时电流（毫安）。符号由厂商决定：P1Max 上充电时是负数，
    // 判断充放电请用 IsCharging。
    public static int GetBatteryCurrentNow()
    {
        try
        {
            android.os.BatteryManager bm =
                (android.os.BatteryManager) sContext.getSystemService(Context.BATTERY_SERVICE);
            if (bm == null)
            {
                return -1;
            }
            int microAmps = bm.getIntProperty(android.os.BatteryManager.BATTERY_PROPERTY_CURRENT_NOW);
            // 不支持的设备会返回 Integer.MIN_VALUE
            if (microAmps == Integer.MIN_VALUE)
            {
                return -1;
            }
            return microAmps / 1000;
        }
        catch (Throwable t)
        {
            return -1;
        }
    }

    // 电池健康，用平台常量：
    // 1 未知 / 2 良好 / 3 过热 / 4 损坏 / 5 过压 / 6 故障 / 7 过冷
    public static int GetBatteryHealth()
    {
        try
        {
            android.content.IntentFilter filter =
                new android.content.IntentFilter(android.content.Intent.ACTION_BATTERY_CHANGED);
            android.content.Intent status = sContext.registerReceiver(null, filter);
            if (status == null)
            {
                return -1;
            }
            return status.getIntExtra(android.os.BatteryManager.EXTRA_HEALTH, -1);
        }
        catch (Throwable t)
        {
            return -1;
        }
    }

    // ------------------------------------------------------------------
    // 系统事件监听（屏幕亮灭、网络变化）
    // ------------------------------------------------------------------
    // 这是本模块唯一"Java 调 C++"的入口：下面的 native 方法在
    // XRDeviceKitEventSubsystem.cpp 里实现，JVM 按名字绑定，
    // 只改一边不改另一边会在运行时崩。

    public static native void OnScreenStateChanged(boolean isOn);

    public static native void OnNetworkChanged(boolean isConnected);

    private static android.content.BroadcastReceiver sScreenReceiver;
    private static android.net.ConnectivityManager.NetworkCallback sNetworkCallback;

    // 开始监听屏幕亮灭，重复调用安全。
    public static void StartScreenWatcher()
    {
        try
        {
            if (sScreenReceiver != null || sContext == null)
            {
                return;
            }
            sScreenReceiver = new android.content.BroadcastReceiver()
            {
                @Override
                public void onReceive(android.content.Context context, android.content.Intent intent)
                {
                    try
                    {
                        String action = intent == null ? null : intent.getAction();
                        if (android.content.Intent.ACTION_SCREEN_ON.equals(action))
                        {
                            OnScreenStateChanged(true);
                        }
                        else if (android.content.Intent.ACTION_SCREEN_OFF.equals(action))
                        {
                            OnScreenStateChanged(false);
                        }
                    }
                    catch (Throwable t)
                    {
                        // 异常绝不能漏回系统
                    }
                }
            };
            android.content.IntentFilter filter = new android.content.IntentFilter();
            filter.addAction(android.content.Intent.ACTION_SCREEN_ON);
            filter.addAction(android.content.Intent.ACTION_SCREEN_OFF);
            sContext.registerReceiver(sScreenReceiver, filter);
        }
        catch (Throwable t)
        {
        }
    }

    public static void StopScreenWatcher()
    {
        try
        {
            if (sScreenReceiver != null && sContext != null)
            {
                sContext.unregisterReceiver(sScreenReceiver);
            }
        }
        catch (Throwable t)
        {
        }
        sScreenReceiver = null;
    }

    // 开始监听默认网络变化，需要 ACCESS_NETWORK_STATE 权限。
    public static void StartNetworkWatcher()
    {
        try
        {
            if (sNetworkCallback != null || sContext == null)
            {
                return;
            }
            android.net.ConnectivityManager cm =
                (android.net.ConnectivityManager) sContext.getSystemService(Context.CONNECTIVITY_SERVICE);
            if (cm == null)
            {
                return;
            }
            sNetworkCallback = new android.net.ConnectivityManager.NetworkCallback()
            {
                @Override
                public void onAvailable(android.net.Network network)
                {
                    try
                    {
                        OnNetworkChanged(true);
                    }
                    catch (Throwable t)
                    {
                    }
                }

                @Override
                public void onLost(android.net.Network network)
                {
                    try
                    {
                        OnNetworkChanged(false);
                    }
                    catch (Throwable t)
                    {
                    }
                }
            };
            cm.registerDefaultNetworkCallback(sNetworkCallback);
        }
        catch (Throwable t)
        {
        }
    }

    public static void StopNetworkWatcher()
    {
        try
        {
            if (sNetworkCallback != null && sContext != null)
            {
                android.net.ConnectivityManager cm =
                    (android.net.ConnectivityManager) sContext.getSystemService(Context.CONNECTIVITY_SERVICE);
                if (cm != null)
                {
                    cm.unregisterNetworkCallback(sNetworkCallback);
                }
            }
        }
        catch (Throwable t)
        {
        }
        sNetworkCallback = null;
    }

    // ------------------------------------------------------------------
    // 内部辅助函数
    // ------------------------------------------------------------------

    // 取某个包的 PackageInfo，不存在或不可见时返回 null。
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
