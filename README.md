# ayypex

A cheat for Apex Legends, for use with Proton on Linux.
It is internal, using a Vulkan layer to render directly on the game's window, like how MangoHud does it.

## Features

Currently, ayypex has:

* Wallhack (box ESP)
* Glow (enemies change color if they are visible)
* Recoil control
* Aim Assist with prediction (sway compensation is broken though)

Currently, all the settings are hardcoded. A settings file is a planned feature though.

## Installing

To install the internal version,

* Move the ayypex.json file to /usr/share/vulkan/implicit_layer.d
* Add AYYPEX=1 %command% to launch options in Steam
* Compile with make
* Install with make install
* Run the game.

## Porting to external
I haven't done this! But it should work...

Files that need to be changed:
* src/core/mem.h (Replace mem::read and mem::write)

Next, you'll need a way to call:
* core::tick(float delta_time) (delta_time should be in seconds)
* core::draw()                 (You'll need to write some kind of overlay with ImGui)
* core::init()                 (Simply call this when your cheat starts.)

## Planned features
(In order of importance)

* Settings (that can be reloaded at runtime)
* Triggerbot
* Spectator count
* Item glow
* Optimize entitylist reading (not really needed since we're internal, but it's nice to have)
* Netvar dumper
* Use mouse movement instead of writing to viewangles

## Bugs

* The game crashes when leaving a match. (Probably due to aimassist? It started doing that after I re-enabled it...)
* There is a small chance you will crash when entering a match. (I have no idea why)

## Credits

* MangoHud for their Vulkan layer code, which was modified and shortened for this project.
* The UnKnoWnCheaTs forum.
