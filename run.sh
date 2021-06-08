#!/bin/bash

echo
echo compiling...
gcc -lX11 -lstdc++ ./zwin.cpp
echo done compiling.

XEPHYR=$(whereis -b Xephyr | cut -f2 -d' ')
xinit ./xinitrc -- \
    "$XEPHYR" \
        :101 \
        -ac \
        -screen 800x600 \
        -host-cursor
