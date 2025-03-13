#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>

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

// Helper class for disabling echo on the terminal
class NoechoInst {
public:
    NoechoInst()
    {
        // If stdin is a tty, save the current settings and disable echo
        if (StdinTerminal()) {
            tcgetattr(STDIN_FILENO, &oldattr);
            struct termios newattr = oldattr;
            newattr.c_lflag &= ~ECHO;
            tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
        }
    }

    ~NoechoInst()
    {
        // Restore terminal settings if needed
        if (StdinTerminal())
            tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
    }

private:
    struct termios oldattr;
}; 