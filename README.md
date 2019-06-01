## Automata Speedrun Mod
Mod that aims to make the NieR: Automata speedrun more tolerable.

As of now all it does is give VC3 mats after adam pit.

### Installation
* Download a version from [releases](https://github.com/jackalstomper/AutomataSpeedrunMod/releases/latest)
* Unzip `AutomataMod.zip`
* Place xinput1_3.dll in the same directory as `NieRAutomata.exe`

Done! The mod should add VC3 mats after you beat Adam 1 bossfight

Note that right now the mod only adds the materials **if you started from a new game!**

### How does it work?
The mod uses a *proxy DLL* of the official Windows DLL `xinput1_3.dll` which gives the mod access to Automata's process memory to modify as it pleases.

The mod still forwards the actual xinput methods to the real windows DLL. The mod just abuses the fact automata loads this DLL to get it's code running first.


### The mod broke!
The mod should show an error box with some details on what went wrong for unrecoverable errors.

There should also be a logfile that the mod generates in your users AppData directory. Example path:
```
C:\Users\<username>\AppData\Roaming\.automataMod\automataMod.log
```