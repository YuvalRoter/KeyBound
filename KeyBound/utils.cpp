#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <fstream> 
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

void setTextColor(unsigned short color) {
    static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    if (!g_colorsEnabled) {
        
        SetConsoleTextAttribute(hConsole, 7);
        return;
    }

    SetConsoleTextAttribute(hConsole, color);
}

bool openFileForRead(const std::string& filename,
    std::ifstream& file,
    const std::string& what)
{
    file.open(filename);
    if (!file) {
        std::cerr << "Error: cannot open " << what << " file: " << filename << std::endl;
        return false;
    }
    return true;
}

