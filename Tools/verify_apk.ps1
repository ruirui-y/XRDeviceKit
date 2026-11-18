# 打包产物检查：确认 Java 侧符号真的进了 APK，以及 UPL 确实被 UAT 收集。
# 用法：
#   pwsh Tools/verify_apk.ps1
#   pwsh Tools/verify_apk.ps1 -Apk "C:\path\to\game.apk"
# 退出码 0 = 全过，1 = 有缺失。

param(
    [string]$Apk = '',
    [string]$ProjectRoot = ''
)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.IO.Compression.FileSystem

if ([string]::IsNullOrEmpty($ProjectRoot))
{
    $ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
}

if ([string]::IsNullOrEmpty($Apk))
{
    $Apk = Join-Path $ProjectRoot 'Binaries\Android\XRDeviceKitDemo-arm64.apk'
}

# 注入进 GameActivity 的方法
$GameActivitySymbols = @(
    'XRDeviceKit_IsScreenOn',
    'XRDeviceKit_GetBrightness',
    'XRDeviceKit_SetBrightnessPercent'
)

# 独立 Java 模块里的方法
$BridgeSymbols = @(
    'com/xrdevicekit/bridge/XRBridge',
    'GetPackageName', 'GetVersionName', 'GetVersionCode', 'IsAppInstalled',
    'GetScreenWidth', 'GetScreenHeight', 'GetScreenDpi', 'GetScreenDensity', 'GetScreenRefreshRate',
    'GetInternalFreeMegabytes', 'GetInternalTotalMegabytes', 'GetExternalFreeMegabytes', 'GetExternalTotalMegabytes',
    'GetUptimeSeconds', 'GetTimeZoneId', 'GetLanguageTag',
    'GetManufacturer', 'GetModel', 'GetDeviceName', 'GetSupportedABIs',
    'GetFirstInstallTimeMs', 'GetLastUpdateTimeMs', 'GetAppLabel', 'StartApp',
    'GetMaxVolume', 'SetVolume', 'GetSystemBrightness', 'IsNightMode'
)

$Failures = @()

Write-Host "APK : $Apk"
if (-not (Test-Path $Apk))
{
    Write-Host "APK 不存在" -ForegroundColor Red
    exit 1
}

# ---- 1. 把所有 dex 的字符串拼起来 ----
$Zip = [System.IO.Compression.ZipFile]::OpenRead($Apk)
try
{
    $DexEntries = $Zip.Entries | Where-Object { $_.FullName -like 'classes*.dex' }
    Write-Host ("dex : " + $DexEntries.Count + " 个")

    $AllText = New-Object System.Text.StringBuilder
    foreach ($Entry in $DexEntries)
    {
        $Stream = $Entry.Open()
        $Memory = New-Object System.IO.MemoryStream
        $Stream.CopyTo($Memory)
        $Stream.Close()
        # dex 的字符串池是 UTF-8 变体，ASCII 名字可以直接按字节搜
        [void]$AllText.Append([System.Text.Encoding]::ASCII.GetString($Memory.ToArray()))
        $Memory.Dispose()
    }
    $Haystack = $AllText.ToString()
}
finally
{
    $Zip.Dispose()
}

# ---- 2. 逐个符号检查 ----
function Test-Symbols
{
    param([string]$Title, [string[]]$Symbols)

    Write-Host ""
    Write-Host "== $Title =="
    foreach ($Symbol in $Symbols)
    {
        if ($Haystack.Contains($Symbol))
        {
            Write-Host ("  [OK]   " + $Symbol) -ForegroundColor Green
        }
        else
        {
            Write-Host ("  [缺失] " + $Symbol) -ForegroundColor Red
            $script:Failures += $Symbol
        }
    }
}

Test-Symbols -Title 'GameActivity 注入的方法' -Symbols $GameActivitySymbols
Test-Symbols -Title 'Java 模块 XRBridge' -Symbols $BridgeSymbols

# ---- 3. UAT 是否收集了我们的 UPL ----
Write-Host ""
Write-Host "== ActiveUPL.txt =="
$UplList = Join-Path $ProjectRoot 'Intermediate\Android\ActiveUPL.txt'
if (-not (Test-Path $UplList))
{
    Write-Host "  [缺失] $UplList（没打过安卓包？）" -ForegroundColor Red
    $Failures += 'ActiveUPL.txt'
}
else
{
    $Hit = Select-String -Path $UplList -Pattern 'XRDeviceKit_APL\.xml' -SimpleMatch:$false
    if ($Hit)
    {
        Write-Host "  [OK]   XRDeviceKit_APL.xml 已被收集" -ForegroundColor Green
    }
    else
    {
        Write-Host "  [缺失] XRDeviceKit_APL.xml 不在收集列表里" -ForegroundColor Red
        $Failures += 'XRDeviceKit_APL.xml'
    }
}

# ---- 4. 汇总 ----
Write-Host ""
if ($Failures.Count -eq 0)
{
    Write-Host "全部通过" -ForegroundColor Green
    exit 0
}

Write-Host ("失败 " + $Failures.Count + " 项：" + ($Failures -join ', ')) -ForegroundColor Red
exit 1
