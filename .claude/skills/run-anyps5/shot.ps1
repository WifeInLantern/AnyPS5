# Usage: shot.ps1 -Out <png path>   (captures the running app.exe window; forces it topmost first)
param([Parameter(Mandatory)][string]$Out)
Add-Type -AssemblyName System.Windows.Forms,System.Drawing
Add-Type @"
using System;using System.Runtime.InteropServices;
public class Win{[DllImport("user32.dll")]public static extern bool GetWindowRect(IntPtr h,out R r);[DllImport("user32.dll")]public static extern bool SetWindowPos(IntPtr h,IntPtr a,int x,int y,int cx,int cy,uint f);[StructLayout(LayoutKind.Sequential)]public struct R{public int L,T,Rt,B;}}
"@
$h=(Get-Process app).MainWindowHandle
[Win]::SetWindowPos($h,[IntPtr](-1),0,0,0,0,0x43)|Out-Null; Start-Sleep 3
$r=New-Object Win+R; [Win]::GetWindowRect($h,[ref]$r)|Out-Null
$bm=New-Object Drawing.Bitmap ($r.Rt-$r.L),($r.B-$r.T)
[Drawing.Graphics]::FromImage($bm).CopyFromScreen($r.L,$r.T,0,0,$bm.Size); $bm.Save($Out)
(Get-Process app).MainWindowTitle
