#!/bin/bash

# To run a command inside the window manager, open a terminal and set the DISPLAY environment variable to ":100"

echo compiling...
make
echo done compiling.

# Band-aid solution to the problem of "could not resolve keysym" flooding, once I figure out why that happens this won't be needed
echo
read -p "Press Ctrl+C to abort or press any other key to continue." -n1 -s
echo

XEPHYR=$(whereis -b Xephyr | cut -f2 -d' ')
xinit ./xinitrc -- \
    "$XEPHYR" \
        :100 \
        -ac \
        -screen 1366x768 \
        -host-cursor
