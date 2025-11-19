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
        // here i should add a ifcolorpressed --------------------------------------------x 
        for (int x = 0; x <= MAX_X; ++x) {
            char c = screen[y][x];

            if (c == WALL) {               // wall character from level1.txt
                setTextColor(Brown);
                std::cout << (char)MediumBlock;
                setTextColor(LightGray);            // reset
            }

            else if (c == WONCHAR) {      // win tile from level1.txt
                setTextColor(LightRed);
                std::cout << (char)DarkBlock;
                setTextColor(LightGray);            // reset
            }
          
            else {
                setTextColor(LightGray);
                std::cout << c;
            }
        }
    }
}

void Screen::drawSimonSquare(int left, int top,
    int normalColor) const
{
    setTextColor(normalColor);
    const int SIZE = 6;

    for (int dy = 0; dy < SIZE; ++dy) {
        gotoxy(left, top + dy);
        for (int dx = 0; dx < SIZE; ++dx) {
            std::cout << ':';       // fill the block
        }
    }

    setTextColor(LightGray);//reset to player color
}

void Screen::drawSimon(int litIndex) const
{
    const int SQUARE_SIZE = 6;
    const int GAP = 2;  // space between squares

    const int totalWidth = 2 * SQUARE_SIZE + GAP;
    const int totalHeight = 2 * SQUARE_SIZE + GAP;

    const int startX = (MAX_X - totalWidth) / 2;
    const int startY = (MAX_Y - totalHeight) / 2;

    int x0 = startX;
    int x1 = startX + SQUARE_SIZE + GAP;
    int y0 = startY;
    int y1 = startY + SQUARE_SIZE + GAP;

    // colors for each square (base + highlight)
    struct Square {
        int x, y;
        int baseColor;
        int hiColor;
    };

    Square sq[4] = {
        { x0, y0, Green,      Black  },  // 0: top-left - for user 1
        { x1, y0, Yellow,     Black       },  // 1: top-right - for user 2
        { x0, y1, Red,        Black    },  // 2: bottom-left - for user 3
        { x1, y1, Blue,       Black   }   // 3: bottom-right- for user 4
    };

    for (int i = 0; i < 4; ++i) {
        int color = (i == litIndex ? sq[i].hiColor : sq[i].baseColor);
        drawSimonSquare(sq[i].x, sq[i].y, color);
    }
}







void Screen::saveBackup()
{
    std::memcpy(backup, screen, sizeof(screen));
    hasBackup = true;
}



void Screen::restoreBackup()
{
    if (hasBackup) {
        std::memcpy(screen, backup, sizeof(screen));
    }
}