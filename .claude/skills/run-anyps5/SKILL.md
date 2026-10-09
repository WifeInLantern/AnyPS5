---
name: run-anyps5
description: Run, launch, start or screenshot a PS5 game with the AnyPS5 build on Windows. Unwraps the game's SELF files, converts it with the relinker, lays out libs/app0, launches app.exe and captures the window. Use when asked to "run <game> with anyps5", "boot the game", "take a screenshot of the game".
---

# Run a game with AnyPS5 (Windows)

AnyPS5 converts a decrypted PS5 executable into a Windows `app.exe` (relinker) that runs against the built system libraries in `libs/`. The driver does the whole pipeline; paths below are relative to the repo root (`E:\Anyps5`).

Use the **PowerShell** tool, and quote game paths (they contain `[` `]`). In Git Bash an unquoted `run\x` loses the backslash and lands in `E:\Anyps5\runx`.

## Prerequisites (all verified present here)

- Built tree: `build\core\relinker\relinker.exe`, `build\core\libs\libs\*.prx`, and the MinGW DLLs in `build\tests\` (`libgcc_s_seh-1.dll`, `libstdc++-6.dll`, `libwinpthread-1.dll`). Build per `docs\dev\BUILD.md` if missing.
- `python` on PATH.
- A **decrypted** game. Its `eboot.bin` / `.prx` may still be SELF-wrapped (magic `4f153d1d` or `5414f5ee`); the relinker rejects that with `The input is a SELF container, not an ELF`. The driver's `unself.py` pulls the plain ELF out (segments are not encrypted in decrypted dumps).

## Run (agent path)

```powershell
.\.claude\skills\run-anyps5\run-game.ps1 -GameDir 'E:\PS5 Games\Dreaming Sarah\PPSA02929-app0' -Out 'run\sarah2' -Seconds 45
.\.claude\skills\run-anyps5\shot.ps1 -Out 'E:\Anyps5\run\sarah2\shot.png'
Stop-Process -Name app -Force
```

`run-game.ps1` unwraps `eboot.bin` and `sce_module\*.prx`, runs `relinker --windows --registry`, fills `<Out>\libs`, junctions every game sub-directory into `<Out>\app0`, copies loose root files (`data.js`, ...), starts `app.exe` with logs in `<Out>\out.log` / `err.log`, and prints `RUNNING pid=...` or `EXITED` plus the first error lines. `shot.ps1` forces the window topmost and screenshots it (the window title shows FPS). Read the PNG to confirm; Dreaming Sarah shows its title menu at ~18 FPS.

Title modules outside `sce_module\` (e.g. `Data\PS5_modules\*.prx`) are passed with `-ExtraModuleDirs <dir>`; they are converted into `app0\prx\*.guest.prx`.

## Human path

Convert by hand per `docs\user\USAGE.md`, then double-click `app.exe`. Set `ANYPS5_GPU=<part of device name>` to pick a Vulkan device.

## Gotchas

- `Test-Path`/`New-Item -Target` treat `[` `]` as wildcards; the driver uses `-LiteralPath` and `cmd /c mklink /J`.
- Without the three MinGW DLLs beside `app.exe` **and** in `libs\`, startup fails with `Failed to load module ...libSceAgc.prx  GetLastError: 126`.
- `GetLastError: 127` = a `.prx` imports a NID that no built library exports. Not a layout problem; run `python tools\import_audit.py <Out>\app.registry.json --libs build\core\libs\libs --modules <Out>\src\sce_module` (without `-I`, it imports a sibling module) to list `absent`/`stub` imports.
- Screenshots: `CopyFromScreen` captures whatever is on top, so `shot.ps1` sets the window topmost first. `SetForegroundWindow` alone is blocked by Windows.
- Starting `app.exe` from a Bash pipe (`... | tail`) never returns while the game runs; use the PowerShell tool.
- `Remove-Item -Recurse` on `<Out>` follows nothing, but remove the `app0` junctions with `cmd /c rmdir` first so the game data is never at risk.

## Known status

- **Dreaming Sarah (PPSA02929)**: boots, renders the menu.
- **Welcome to ParadiZe (PPSA09258)**: converts, then fails at load with `unresolved ELF import zw+celG7zSI from libSceRazorCpu.prx` (160 absent / 51 stub imports in the audit). Its `Data\Prx\EOSSDK-PS5-Shipping.prx` is also demanded as `libs\EOSSDK-PS5-Shipping.debug_prx`; copying the converted `.guest.prx` there got past that. Needs emulator-side work, not a run-skill fix.
