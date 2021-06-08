#!/bin/bash

# To run a command inside the window manager, open a terminal and set the DISPLAY environment variable to ":100"

echo
echo compiling...
gcc -lX11 -lstdc++ -g ./zwin.cpp
echo done compiling.

XEPHYR=$(whereis -b Xephyr | cut -f2 -d' ')
xinit ./xinitrc -- \
    "$XEPHYR" \
        :100 \
        -ac \
        -screen 1600x900 \
        -host-cursor
