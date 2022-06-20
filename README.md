## Automata Speedrun Mod

Mod that aims to make the NieR: Automata speedrun more tolerable.

As of now all it does is give VC3 mats after adam pit.

### Installation

- Download a version from [releases](https://github.com/jackalstomper/AutomataSpeedrunMod/releases/latest)
- Unzip `AutomataMod.zip`
- Place xinput1_4.dll in the same directory as `NieRAutomata.exe`

Done! The mod should add VC3 mats after you beat Adam 1 bossfight

Note that right now the mod only adds the materials **if you started from a new game!**

### How does it work?

The mod uses a _proxy DLL_ of the official Windows DLL `xinput1_4.dll` which gives the mod access to Automata's process memory to modify as it pleases.

The mod still forwards the actual xinput methods to the real windows DLL. The mod just abuses the fact automata loads this DLL to get it's code running first.

### The mod broke!

The mod should show an error box with some details on what went wrong for unrecoverable errors.

There should also be a logfile that the mod generates in your users AppData directory. Example path:

```
C:\Users\<username>\AppData\Roaming\.automataMod\automataMod.log
```

This directory can be accessed quickly by opening the `Run` window (Winkey + R) and typing in `%appdata%`

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

### Contributors

- [icefire](https://github.com/jackalstomper) - Author
- Martino - Researching and providing an example implementation of the watermark Direct2D code
- [remote-mine](https://github.com/remote-mine) - Adding fishing replacement logic for Mackerel runs
- [DisrespectDwardo](https://github.com/DisrespectDwardo) - Authoring code for the FPS display
