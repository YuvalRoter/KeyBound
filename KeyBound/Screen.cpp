#include <iostream>

#include "Screen.h"
#include "utils.h"
#include <fstream> 


bool Screen::loadFromFile(const std::string& filename)// פונקציה שמנסה לעלות את המפה למסך ומחזירה אמת אם היא הצליחה
{
    std::ifstream file(filename);

    if (!file) {
        std::cerr << "Error: cannot open map file: " << filename << std::endl;
        return false;
    }

    std::string line; // משתנה לאחסון שורות מהקובץ

    for (int y = 0; y <= MAX_Y; ++y) {
        if (!std::getline(file, line)) { // קריאה שורה מהקובץ
            std::cerr << "Error: not enough lines in map file: " << filename << std::endl;
            return false;
        }

        for (int x = 0; x <= MAX_X; ++x) {
            screen[y][x] = line[x];
        }
        screen[y][MAX_X + 1] = '\0';
    }


    
    return true;
}

void Screen::draw() const {
    for (int y = 0; y <= MAX_Y; ++y) {
        gotoxy(0, y);

        for (int x = 0; x <= MAX_X; ++x) {
            char c = screen[y][x];

            if (c == 'W') {                 // wall character from level1.txt
                setTextColor(6);            // Dark Yellow
                std::cout << (char)176;     // █
                setTextColor(7);            // reset
            }
            else if (c == '%') {            // win tile from level1.txt
                setTextColor(10);           // Bright Green
                std::cout << (char)178;     // █
                setTextColor(7);            // reset
            }
            else {
                setTextColor(7);
                std::cout << c;
            }
        }
    }
}
