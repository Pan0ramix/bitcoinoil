// Copyright (c) 2022-2023 The Bitcoinoil developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOINOIL_CLI_UTILS_H
#define BITCOINOIL_CLI_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>

// Check if standard input has data available
bool StdinReady();

// Check if standard input is connected to a terminal
bool StdinTerminal();

// Helper class for disabling echo on the terminal
class NoechoInst {
public:
    NoechoInst();
    ~NoechoInst();

private:
    struct termios oldattr;
};

#endif // BITCOINOIL_CLI_UTILS_H 