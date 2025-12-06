#include <iostream>
#include "Screen.h"
#include "utils.h"
#include <fstream> 
#include "Room.h"
#include <windows.h> 


bool Screen::loadFromFileToMap(const std::string& filename)
{
    std::ifstream file;


    if (!openFileForRead(filename, file, "map"))
        return false;

    std::string line;


    for (int y = 0; y <= MAX_Y; ++y) {

        if (!std::getline(file, line)) {
            line = "";
        }

        for (int x = 0; x <= MAX_X; ++x) {
            if (x < line.length()) {
                screen[y][x] = line[x];
            }
            else {
                screen[y][x] = ' ';
            }
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

void Screen::drawSimonSquare(int left, int top, char filler) const
{
    const int WIDTH = SIMON_WIDTH;
    const int HEIGHT = SIMON_HEIGHT;

    for (int dy = 0; dy < HEIGHT; ++dy) {
        gotoxy(left, top + dy);
        for (int dx = 0; dx < WIDTH; ++dx) {
            std::cout << filler;
        }
    }

    setTextColor(LightGray); // reset
}



void Screen::drawSimon(int litIndex) const
{
    const int WIDTH = SIMON_WIDTH;
    const int HEIGHT = SIMON_HEIGHT;
    const int GAP_X = SIMON_GAP_X;
    const int GAP_Y = SIMON_GAP_Y;

    const int totalWidth = 2 * WIDTH + GAP_X;
    const int totalHeight = 2 * HEIGHT + GAP_Y;

    // +1 because coordinates are 0..MAX_X
    const int startX = ((MAX_X + 1) - totalWidth) / 2;
    const int startY = ((MAX_Y + 1) - totalHeight) / 2;

    int x0 = startX;
    int x1 = startX + WIDTH + GAP_X;
    int y0 = startY;
    int y1 = startY + HEIGHT + GAP_Y;

    // colors for each square (base + highlight)
    struct Square {
        int x, y;
        int baseColor;
        int hiColor;
        int BeepNumber;
    };

    Square sq[4] = {
        { x0, y0, Green,      Black, 330  },  // 0: top-left - for user 1
        { x1, y0, Yellow,     Black, 440       },  // 1: top-right - for user 2
        { x0, y1, Red,        Black, 523    },  // 2: bottom-left - for user 3
        { x1, y1, Blue,       Black,659   }   // 3: bottom-right- for user 4
    };

    for (int i = 0; i < 4; ++i) {
        bool isLit = (i == litIndex);
        int color = isLit ? sq[i].hiColor : sq[i].baseColor;

        setTextColor(color);

        if (g_colorsEnabled) {
            drawSimonSquare(sq[i].x, sq[i].y, char(219)); // solid block
        }
        else { // color-blind mode
            if (color == Black)
                drawSimonSquare(sq[i].x, sq[i].y, ' ');
            else
                drawSimonSquare(sq[i].x, sq[i].y, char(219));
        }

        if (isLit) {
            Beep(sq[i].BeepNumber, 250); 
        }
        
    }
}



void Screen::saveScreenToRoom(Room& room) {
    room.savedMapState.clear(); // Clear old state

    for (int y = 0; y <= MAX_Y; ++y) {
        // Convert the char array row into a std::string
        // We use the string constructor that takes a char*
        room.savedMapState.push_back(std::string(screen[y]));
    }

    room.isVisited = true; // Mark that we have valid data for this room
}

void Screen::loadScreenFromRoom(const Room& room) {
    // Safety check
    if (!room.isVisited) return;

    for (int y = 0; y <= MAX_Y; ++y) {
        // Check if the saved state has this row
        if (y < room.savedMapState.size()) {
            std::string line = room.savedMapState[y];

            // Copy string back to char array
            for (int x = 0; x <= MAX_X; ++x) {
                if (x < line.size()) {
                    screen[y][x] = line[x];
                }
                else {
                    screen[y][x] = ' '; // Fill remaining width with spaces
                }
            }
        }
        else {
            // If saved state has fewer rows than screen, fill with spaces
            for (int x = 0; x <= MAX_X; ++x) {
                screen[y][x] = ' ';
            }
        }
        screen[y][MAX_X + 1] = '\0'; // Null-terminate the C-string
    }
}




void Screen::saveBackup()
{
    //std::cout << "[DEBUG] saveBackup: char(0,0) = " << screen[0][0] << "\n"; - for debug
    std::memcpy(backup, screen, sizeof(screen));
    hasBackup = true;
}



void Screen::restoreBackup()
{
    if (hasBackup) {
        std::memcpy(screen, backup, sizeof(screen));
        //std::cout << "[DEBUG] restoreBackup: char(0,0) = " << screen[0][0] << "\n"; - for debug 
    }
    
}