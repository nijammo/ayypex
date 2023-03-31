# ayypex

A cheat for Apex Legends, for use with Proton on Linux.
It is internal, using a Vulkan layer to render directly on the game's window.
The cheat itself should be portable, meaning that if you can provide it with a on-draw and on-tick function then it should be able to also run externally.

## Features

Currently, ayypex has

* Wallhack
* Aim Assist

The aim assist might be a bit too aggressive, you might want to modify the PID controller coefficients in core/modules/aimassist.h

## Installing

To install the internal version

* Move the ayypex.json file to /usr/share/vulkan/implicit_layer.d
* Add AYYPEX=1 %command% to launch options in Steam
* Compile with make
* Install with make install
* Run the game.

## Credits

Most of the code for the Vulkan layer was taken from Mesa's Vulkan overlay project, but modified for usage on projects like this.