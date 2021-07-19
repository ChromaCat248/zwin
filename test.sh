#!/bin/bash

# To run a command inside the window manager, open a terminal and set the DISPLAY environment variable to ":100"

echo
echo compiling...
make
echo done compiling.

XEPHYR=$(whereis -b Xephyr | cut -f2 -d' ')
xinit ./xinitrc -- \
    "$XEPHYR" \
        :100 \
        -ac \
        -screen 1366x768 \
        -host-cursor
