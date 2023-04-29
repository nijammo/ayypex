# ayypex

A cheat for Apex Legends, for use with Proton on Linux.
It is internal, using a Vulkan layer to render directly on the game's window.
The cheat itself should be portable, meaning that if you can provide it with a on-draw, on-tick function and change the memory interface then it should be able to also run externally.

## Features

Currently, ayypex has

* Wallhack
* Glow
* Recoil control
* Aim Assist with prediction (sway compensation is broken though)

## Installing

To install the internal version

* Move the ayypex.json file to /usr/share/vulkan/implicit_layer.d
* Add AYYPEX=1 %command% to launch options in Steam
* Compile with make
* Install with make install
* Run the game.

## Planned features

* Triggerbot
* Settings (that can be reloaded at runtime)
* Spectator count
* Optimize entitylist reading

## Credits

* MangoHud for their Vulkan layer code, which was modified and shortened for this project.
* The UnKnoWnCheaTs forum.
