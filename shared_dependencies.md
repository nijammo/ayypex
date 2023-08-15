Shared Dependencies:

1. Namespace: "core" - This namespace is shared across all the files and functions.
2. Function: "mem::read" and "mem::write" - These functions are shared in the file "src/core/mem.h".
3. Function: "core::tick" - This function is shared in the file "core/tick.py". It uses the variable "delta_time".
4. Function: "core::draw" - This function is shared in the file "core/draw.py". It requires an overlay with ImGui.
5. Function: "core::init" - This function is shared in the file "core/init.py". It is called when the cheat starts.
6. Variable: "delta_time" - This variable is shared in the function "core::tick". It should be in seconds.
7. Library: "ImGui" - This library is shared in the function "core::draw" for creating an overlay.