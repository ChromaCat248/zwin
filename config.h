// ZWin Config File

#pragma once

// gap sizes
const unsigned int outerGap = 10;
const unsigned int innerGap = 10;

// visual properties of frame
const unsigned int borderWidth = 1;
const unsigned int barHeight = 25;
const unsigned int barGap = 2;
const unsigned long borderColor = 0x888888;
const unsigned long backgroundColor = 0x444444;
const unsigned long barColor = 0x222222;

const bool exitOnRCError = false;

int rc() {
    // Add functions in here you want to run on startup

    // If this function returns anything other than 0 and exitOnRCError is set to true, ZWin will quit
    return 0;
};
