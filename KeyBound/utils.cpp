#include <windows.h>
#include <iostream>
#include <cstdlib>
#include "utils.h"

bool g_colorsEnabled = false;

void gotoxy(int x, int y) {
    std::cout.flush();
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void hideCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(consoleHandle, &cursorInfo);
    cursorInfo.bVisible = FALSE; // Hide the cursor
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);
}

void cls() {
    system("cls");
}

void setTextColor(WORD color) {
    static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    if (!g_colorsEnabled) {
        
        SetConsoleTextAttribute(hConsole, 7);
        return;
    }

    SetConsoleTextAttribute(hConsole, color);
}



