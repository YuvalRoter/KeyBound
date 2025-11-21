#pragma once
#include <windows.h> 

void gotoxy(int x, int y);

void hideCursor();

void cls();

void setTextColor(WORD color);

extern bool g_colorsEnabled;
bool openFileForRead(const std::string& filename,
    std::ifstream& file,
    const std::string& what);
