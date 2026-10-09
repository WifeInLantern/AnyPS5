# Usage: run-game.ps1 -GameDir <dir with eboot.bin> -Out run\<name> [-ExtraModuleDirs dir,...] [-Seconds 30]
param([Parameter(Mandatory)][string]$GameDir,[Parameter(Mandatory)][string]$Out,[string[]]$ExtraModuleDirs=@(),[int]$Seconds=30)
$ErrorActionPreference='Stop'
$root=(Resolve-Path "$PSScriptRoot\..\..\..").Path
$unself="$PSScriptRoot\unself.py"
New-Item -ItemType Directory -Force "$Out\src\sce_module","$Out\src\prx","$Out\libs","$Out\app0" | Out-Null
$Out=(Resolve-Path $Out).Path
python -I $unself (Join-Path $GameDir eboot.bin) "$Out\src\eboot.elf"
foreach($m in Get-ChildItem -LiteralPath (Join-Path $GameDir sce_module) -File -Filter *.prx -ErrorAction SilentlyContinue){ python -I $unself $m.FullName "$Out\src\sce_module\$($m.Name)" }
foreach($dir in $ExtraModuleDirs){ foreach($m in Get-ChildItem -LiteralPath $dir -File -Filter *.prx){ python -I $unself $m.FullName "$Out\src\prx\$($m.Name)" } }
if(-not (Get-ChildItem "$Out\src\prx")){ Remove-Item "$Out\src\prx" }
& "$root\build\core\relinker\relinker.exe" --windows --registry "$Out\src\eboot.elf" "$Out\app.exe" | Select-Object -Last 3
if($LASTEXITCODE){ throw "relinker exit $LASTEXITCODE" }
Copy-Item "$root\build\core\libs\libs\*.prx" "$Out\libs\"
foreach($f in 'libgcc_s_seh-1.dll','libstdc++-6.dll','libwinpthread-1.dll'){ Copy-Item "$root\build\tests\$f" $Out; Copy-Item "$root\build\tests\$f" "$Out\libs\" }
# game resources: junction every top-level dir (cmd mklink: PowerShell treats [ ] in paths as wildcards)
foreach($d in Get-ChildItem -LiteralPath $GameDir -Directory){ if($d.Name -ne 'sce_module' -and -not (Test-Path -LiteralPath "$Out\app0\$($d.Name)")){ cmd /c mklink /J "$Out\app0\$($d.Name)" $d.FullName | Out-Null } }
foreach($f in Get-ChildItem -LiteralPath $GameDir -File){ $t="$Out\app0\$($f.Name)"; if($f.Name -notlike 'eboot.bin*' -and -not (Test-Path -LiteralPath $t)){ Copy-Item -LiteralPath $f.FullName $t } }
$p=Start-Process "$Out\app.exe" -WorkingDirectory $Out -RedirectStandardOutput "$Out\out.log" -RedirectStandardError "$Out\err.log" -PassThru
Start-Sleep $Seconds
if($p.HasExited){ "EXITED code=$($p.ExitCode)"; Get-Content "$Out\err.log" -TotalCount 6 } else { "RUNNING pid=$($p.Id)" }
