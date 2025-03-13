// Copyright (c) 2022-2023 The Bitcoinoil developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "cli-utils.h"

// Check if standard input has data available
bool StdinReady()
{
    struct timeval timeout;
    fd_set fds;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &timeout) == 1;
}

// Check if standard input is connected to a terminal
bool StdinTerminal()
{
    return isatty(STDIN_FILENO);
}

// Constructor for NoechoInst class
NoechoInst::NoechoInst()
{
    // If stdin is a tty, save the current settings and disable echo
    if (StdinTerminal()) {
        tcgetattr(STDIN_FILENO, &oldattr);
        struct termios newattr = oldattr;
        newattr.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    }
}

// Destructor for NoechoInst class
NoechoInst::~NoechoInst()
{
    // Restore terminal settings if needed
    if (StdinTerminal())
        tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
} 