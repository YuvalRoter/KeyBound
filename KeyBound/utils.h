#pragma once
#include <string>
#include <fstream>


void gotoxy(int x, int y);
void hideCursor();
void cls();


void setTextColor(unsigned short color);

extern bool g_colorsEnabled;

bool openFileForRead(const std::string& filename,
    std::ifstream& file,
    const std::string& what);