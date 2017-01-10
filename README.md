sourcemod-nt-speedrun
=====================

### What is this?
It's a SourceMod plugin for Neotokyo servers. It times players as they run between the NSF-Jinrai-NSF capzones,
also storing "split times" for each capzone passed, announcing new records and such.

### Does it work?
Kinda! There's a few gotchas though, namely:
* It has to be loaded on mapchange/restart. For now.
* The "teleport to start" function ignores props and walls, so you might get stuck on some maps using it.
This can be worked around by manually removing clutter with the Metamod plugin Stripper or similar.
* It probably shouldn't be run on a "normal server", as it includes things like teleporting and destroying cap points.
* It doesn't (yet) store times over a mapchange/player disconnect! :-(

### Commands
* sm_run (!run) - Opens a menu with option to teleport to the start.

### Compile dependencies
* [Neotokyo include](https://github.com/softashell/sourcemod-nt-include)

### Server requirements
* SourceMod 1.7 or later
