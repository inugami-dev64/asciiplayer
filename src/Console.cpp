//
// Created by user on 25/04/19.
//

#include "Console.h"

#include <cstdio>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#elif defined(__linux__)
#include <sys/ioctl.h>
#include <unistd.h>
#elif defined(__APPLE__)
#error "MacOS is not supported lol, get fucked!"
#endif

namespace ap {
    Rectangle<int> Console::get_console_dimensions() {
#if defined(_WIN32)
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        int cols, rows;

        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

        return Rectangle<int>{cols, rows};
#elif defined(__linux__)
        winsize win{};
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);

        return Rectangle<int>{win.ws_col, win.ws_row};
    }
#endif

    void Console::output(const char* data) {
        printf("%s", data);
    }

    void Console::clear_console() {
#if defined(_WIN32)
        COORD topLeft  = { 0, 0 };
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO screen;
        DWORD written;

        GetConsoleScreenBufferInfo(console, &screen);
        FillConsoleOutputCharacterA(
            console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
        );
        FillConsoleOutputAttribute(
            console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
            screen.dwSize.X * screen.dwSize.Y, topLeft, &written
        );
        SetConsoleCursorPosition(console, topLeft);
#elif defined(__linux__)
        printf("\033[2J\033[H");
#endif
    }

} // ap