# ZenJiggle

A tiny Windows system tray utility that keeps your PC awake — no display sleep, no lock screen, no idle-triggered actions — without moving the mouse or simulating input.

## How it works

ZenJiggle uses the Windows `SetThreadExecutionState` API to tell the OS directly that the system should stay awake (`ES_SYSTEM_REQUIRED` + `ES_DISPLAY_REQUIRED`), refreshed every 30 seconds while active. This is the same mechanism apps like video players use to prevent sleep during playback — it's a legitimate state flag, not a simulated mouse jiggle or keystroke.

## Features

- Lives in the system tray with a status icon (green = active, red = paused)
- Single-instance guard — won't stack duplicate tray icons if launched twice
- Toggle pause/resume via the tray menu or a left double-click on the icon
- Cleanly releases the keep-awake state on pause or exit

## Requirements

- Windows
- [.NET 8 Desktop Runtime](https://dotnet.microsoft.com/download/dotnet/8.0) (or build self-contained, see below)

## Building

```bat
build.bat
```

This runs `build.ps1`, which restores dependencies and publishes a single-file, framework-dependent `win-x64` Release build to `dist\ZenJiggle.exe`.

Custom configuration or runtime:

```powershell
.\build.ps1 -Configuration Release -Runtime win-x64
```

## Running

Launch `dist\ZenJiggle.exe`. The app starts minimized to the tray with keep-awake active immediately. Right-click the tray icon for **Pause**/**Resume** and **Exit**.

## Project structure

| File | Purpose |
|---|---|
| `Program.cs` | Entry point and tray application logic |
| `ZenJiggle.csproj` | .NET 8 Windows Forms project file |
| `app.ico` | Application icon |
| `build.ps1` / `build.bat` | Build/publish scripts |
