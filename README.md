# ZenJiggle (C version)

A tiny Windows system tray utility that keeps your PC awake — no display sleep, no lock screen, no idle-triggered actions — without moving the mouse or simulating input.

This is a plain C rewrite of the .NET version in the parent folder. It has the same features, but the exe is about 16 KB and needs no .NET runtime or any other install.

## How it works

ZenJiggle calls the Windows `SetThreadExecutionState` API with `ES_SYSTEM_REQUIRED` + `ES_DISPLAY_REQUIRED`, and refreshes it every 30 seconds while active. This is the same mechanism video players use to prevent sleep during playback. It does not move the mouse or press keys.

## Features

- Lives in the system tray (green dot = active, red dot = paused)
- Single-instance guard — a second copy exits at once
- Pause/resume from the tray menu or by double-clicking the icon
- Releases the keep-awake state on pause and on exit
- No C runtime, no dependencies except standard Windows DLLs (`kernel32`, `user32`, `shell32`, `gdi32`)

## Requirements

- To run: Windows. Nothing else.
- To build: [MinGW-w64](https://www.mingw-w64.org/) (`gcc` and `windres` on `PATH`).

## Building

```bat
build.bat
```

The output is `dist\ZenJiggle.exe`.

## Running

Launch `dist\ZenJiggle.exe`. It starts in the tray with keep-awake on. Right-click the icon for **Pause**/**Resume** and **Exit**.

> The .NET version uses the same single-instance lock name. Close it before you run this one.

## "Smart App Control blocked an app" on Windows 11

`ZenJiggle.exe` is not code-signed yet. Windows 11 **Smart App Control** (SAC) blocks unsigned apps that it does not know. The app is safe, and the full source is in this repo.

Your options:

1. **Install with PowerShell.** This download does not get the "Mark of the Web" tag, so Windows may allow it. It is not guaranteed.
   ```powershell
   irm https://raw.githubusercontent.com/sarfraznawaz2005/ZenJiggle/main/install.ps1 | iex
   ```
   The script saves the exe to `%LOCALAPPDATA%\ZenJiggle\ZenJiggle.exe` and starts it. You can read it first: [install.ps1](install.ps1).
2. **Build it yourself** (best if SAC is on). Install MinGW-w64, run `build.bat`, and run `dist\ZenJiggle.exe`. You can read every line of code first.
3. **Turn off Smart App Control.** Open *Settings > Privacy & security > Windows Security > App & browser control > Smart App Control settings* and choose **Off**. You cannot turn it on again later without a Windows reset.

Things that do **not** help:

- Downloading from a different place. Files from the internet get a "Mark of the Web" tag, which makes Windows *more* strict, not less.
- Right-click > Properties > Unblock. This only helps with SmartScreen, not Smart App Control.

For the maintainer: the real fix is to sign the exe with a certificate from a trusted authority (for example, Microsoft Artifact Signing, formerly Trusted Signing). A self-signed certificate does not work with SAC.

## Project structure

| File | Purpose |
|---|---|
| `zenjiggle.c` | All the app code |
| `zenjiggle.rc` | Resource file that embeds the exe icon |
| `app.ico` | Exe file icon |
| `build.bat` | Build script |

## How it stays small

- `-nostartfiles` with a custom `WinMainCRTStartup` entry point skips the C runtime.
- `-Os`, `-s`, and `--gc-sections` optimize for size, strip symbols, and drop unused code.
- The tray icons are drawn in code, so no image files are needed.
