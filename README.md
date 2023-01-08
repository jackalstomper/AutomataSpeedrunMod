# Automata Speedrun Mod

Mod that aims to make the NieR: Automata speedrun more tolerable.

## Installation

- Download a version from [releases](https://github.com/jackalstomper/AutomataSpeedrunMod/releases/latest)
- Unzip `AutomataMod.zip`
- Place xinput1_4.dll in the same directory as `NieRAutomata.exe`

Done! The game should now display a VC3Mod watermark on screen when loaded.  
The mod only makes changes **if you start as run from a new game!**

## How does it work?

The mod uses a _proxy DLL_ of the official Windows DLL `xinput1_4.dll` which gives the mod access to Automata's process memory to modify as it pleases.  
The mod still forwards the actual xinput methods to the real windows DLL. The mod just abuses the fact automata loads this DLL to get it's code running first.

## The mod broke!

The mod should show an error box with some details on what went wrong for unrecoverable errors.
There should also be a logfile that the mod generates in your users AppData directory. Example path:

```
C:\Users\<username>\AppData\Roaming\.automataMod\automataMod.log
```

This directory can be accessed quickly by opening the `Run` window (Winkey + R) and typing in `%appdata%`.
Note that this log is reset every time you boot the game! If you want to share this log to help with troubleshooting make sure to save it before rebooting your game again.

Example of a log with the mod functioning as normal

```plain
2019-06-13 15:34:33 [INFO] Initializing AutomataMod
2019-06-13 15:34:33 [INFO] Trying to find system directory to load real xinput DLL
2019-06-13 15:34:33 [INFO] Found directory: <dll directory>
2019-06-13 15:34:33 [INFO] Process ram start: <number>
2019-06-13 15:34:33 [INFO] Assigning procs
2019-06-13 15:36:31 [INFO] Detected we need to change savefile name. Changing.
2019-06-13 15:36:47 [INFO] Detected we are in 58_AB_BossArea_Fall. Giving VC3 inventory
2019-06-13 15:36:47 [INFO] No dented plates found. Adding...
2019-06-13 15:36:47 [INFO] No severed cables found. Adding...
2019-06-13 15:36:47 [INFO] Current Dented Plates: 0
2019-06-13 15:36:47 [INFO] Current Severed Cables: 0
2019-06-13 15:36:47 [INFO] Setting dented plates to 4
2019-06-13 15:36:47 [INFO] Setting severed cables to 3
2019-06-13 15:36:47 [INFO] Current Dented Plates: 4
2019-06-13 15:36:47 [INFO] Current Severed Cables: 3
2019-06-13 15:36:47 [INFO] Done adding inventory.
```

# Development

This project uses CMake for project file generation.

### Visual Studio
The project can be used in Visual Studio by opening it as a [CMake Project](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-170).

### VSCode
The project can be used in VScode with the [CMake tools Extension](https://devblogs.microsoft.com/cppblog/cmake-tools-extension-for-visual-studio-code/)

### Command Line
The project can be built and/or project files generated with direct cmake usage.  
Example:
```shell
mkdir build
cd build
cmake ..
```

### Debugging
With automata 1.02 the game will crash if a debugger is attached due to steam DRM.  
In order to debug while developing the application will need to be unpacked with https://github.com/atom0s/Steamless

The app needs an steam appid to function, these can be added using environment variables.  
Example vscode launch configuration:
```json
{
    "name": "Launch Automata",
    "type": "cppvsdbg",
    "request": "launch",
    "program": "${env:NIER_DIR}\\NieRAutomata.exe.unpacked.exe",
    "cwd": "${env:NIER_DIR}",
    "symbolSearchPath": "${workspaceFolder}/build/Debug",
    "args": [],
    "environment": [
        {
            "name": "SteamGameId",
            "value": "524220"
        },
        {
            "name": "SteamAppId",
            "value": "524220"
        },
        {
            "name": "SteamOverlayGameId",
            "value": "524220"
        }
    ],
    "logging": {
        "moduleLoad": true,
        "trace": true
    }
}
```

### Styling
The project uses a slightly modified LLVM code style. The project is configured to use clang format for automated styling.  
The styling configuration is located in `.clang-format`

### Dependencies
The project has a dependency on the string formatting library [fmt](https://github.com/fmtlib/fmt) that is included as a [git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules).  
The dependency can be added when initially cloning the repository with `git clone --recurse-submodules`.  
If the repo has already been cloned then the submodule can be downloaded with `git submodule update --init --recursive`.

# Contributors

- [icefire](https://github.com/jackalstomper) - Author
- [Martino](https://github.com/Martymoose98) - Researching and providing an example implementation of the watermark Direct2D code, using hash check for executable version.
- [remote-mine](https://github.com/remote-mine) - Adding fishing replacement logic for Mackerel runs
- [DisrespectDwardo](https://github.com/DisrespectDwardo) - Authoring code for the FPS display
