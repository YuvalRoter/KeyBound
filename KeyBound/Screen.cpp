#include <iostream>
#include <fstream> 
#include <cstring>     
#include <windows.h>   
#include "Screen.h"
#include "utils.h"
#include "Room.h"

// ===========================
//      Map Loading
// ===========================

bool Screen::loadFromFileToMap(const std::string& filename)
{
    std::ifstream file;
    if (!openFileForRead(filename, file, "map"))
        return false;

    // Clear buffer (optional but safe)
    for (int y = 0; y <= MAX_Y; ++y) {
        for (int x = 0; x <= MAX_X; ++x) {
            screen[y][x] = ' ';
        }
        screen[y][MAX_X + 1] = '\0';
    }

    std::string line;
    int y = 0;

    while (y <= MAX_Y && std::getline(file, line)) {

        // Skip header lines
        if (!line.empty() && line[0] == '@') {
            continue;
        }

        // Make sure the line is exactly 80 chars (MAX_X+1)
        if ((int)line.size() < MAX_X + 1) {
            line += std::string((MAX_X + 1) - line.size(), ' ');
        }
        else if ((int)line.size() > MAX_X + 1) {
            line = line.substr(0, MAX_X + 1);
        }

        for (int x = 0; x <= MAX_X; ++x) {
            screen[y][x] = line[x];
        }

        screen[y][MAX_X + 1] = '\0';
        ++y;
    }

    return (y == MAX_Y + 1);
}

// ===========================
//        Rendering
// ===========================

void Screen::draw() const {
    for (int y = 0; y <= MAX_Y; ++y) {
        gotoxy(0, y);

        for (int x = 0; x <= MAX_X; ++x) {
            char c = screen[y][x];

            if (c == WALL) {
                setTextColor(Color::Brown);
                std::cout << static_cast<char>(BlockType::MediumBlock);
                setTextColor(Color::LightGray); // Reset
            }
            else if (c == OBSTACLE) {
				setTextColor(Color::DarkGray);
				std::cout << c;
				setTextColor(Color::LightGray); // Reset
            }
            else if (c == WONCHAR) {
                setTextColor(Color::LightRed);
                std::cout << static_cast<char>(BlockType::DarkBlock);
                setTextColor(Color::LightGray); // Reset
            }
            else if (c == PLAYER1) {
                setTextColor(Color::Cyan);
                std::cout << c;
                setTextColor(Color::LightGray); // Reset
            }
            else if (c == PLAYER2) {
                setTextColor(Color::Red);
                std::cout << c;
                setTextColor(Color::LightGray); // Reset
            }
            else if (c == SWITCH_ON) {
                setTextColor(Color::Green);
                std::cout << c;
                setTextColor(Color::LightGray);
            }
            else if (c == SWITCH_OFF) {
                setTextColor(Color::Red);
                std::cout << c;
                setTextColor(Color::LightGray);
            }
            else if (c == TORCH) {
                setTextColor(Color::Yellow);
                std::cout << c;
                setTextColor(Color::LightGray);
            }
            else if (c == KEY) {
                setTextColor(Color::LightCyan);
                std::cout << c;
                setTextColor(Color::LightGray);
            }
            else if (c == '8') {
                setTextColor(Color::Red);
                std::cout << c;
                setTextColor(Color::LightGray);
            }
            else {
                setTextColor(Color::LightGray);
                std::cout << c;
            }
        }
    }
}


// ===========================
//      Simon Says Logic
// ===========================

void Screen::drawSimonSquare(int left, int top, char filler) const
{
    // Use the static constants from the header
    const int WIDTH = SIMON_WIDTH;
    const int HEIGHT = SIMON_HEIGHT;

    for (int dy = 0; dy < HEIGHT; ++dy) {
        gotoxy(left, top + dy);
        for (int dx = 0; dx < WIDTH; ++dx) {
            std::cout << filler;
        }
    }

    setTextColor(Color::LightGray); // Reset to default
}

void Screen::drawSimon(int litIndex) const
{
    // Local aliases for readability
    const int WIDTH = SIMON_WIDTH;
    const int HEIGHT = SIMON_HEIGHT;
    const int GAP_X = SIMON_GAP_X;
    const int GAP_Y = SIMON_GAP_Y;

    const int totalWidth = 2 * WIDTH + GAP_X;
    const int totalHeight = 2 * HEIGHT + GAP_Y;

    // Center the game on the screen
    const int startX = ((MAX_X + 1) - totalWidth) / 2;
    const int startY = ((MAX_Y + 1) - totalHeight) / 2;

    int x0 = startX;
    int x1 = startX + WIDTH + GAP_X;
    int y0 = startY;
    int y1 = startY + HEIGHT + GAP_Y;

    // Structure for the 4 Simon buttons
    struct Square {
        int x, y;
        int baseColor;
        int hiColor;
        int BeepNumber;
    };

    // Initialize the 4 quadrants
    Square sq[4] = {
        { x0, y0, Color::Green,  Color::Black, 330 }, // Top-Left
        { x1, y0, Color::Yellow, Color::Black, 440 }, // Top-Right
        { x0, y1, Color::Red,    Color::Black, 523 }, // Bottom-Left
        { x1, y1, Color::Blue,   Color::Black, 659 }  // Bottom-Right
    };

    for (int i = 0; i < 4; ++i) {
        bool isLit = (i == litIndex);
        int color = isLit ? sq[i].hiColor : sq[i].baseColor;

        setTextColor(color);

        if (g_colorsEnabled) {
            drawSimonSquare(sq[i].x, sq[i].y, static_cast<char>(BlockType::FullBlock));
        }
        else {
            // Color-blind mode support: Lit squares are solid, unlit are empty
            if (color == Color::Black)
                drawSimonSquare(sq[i].x, sq[i].y, ' ');
            else
                drawSimonSquare(sq[i].x, sq[i].y, static_cast<char>(BlockType::FullBlock));
        }

        // Play sound if this is the active square
        if (isLit) {
            Beep(sq[i].BeepNumber, 250);
        }
    }
}

// ===========================
//      State Management
// ===========================


void Screen::saveScreenToRoom(Room& room) const {
    room.savedMapState.clear();

    for (int y = 0; y <= MAX_Y; ++y) {
        // Construct std::string from the char array (relies on null-terminator)
        room.savedMapState.push_back(std::string(screen[y]));
    }

    room.isVisited = true;
}

void Screen::loadScreenFromRoom(const Room& room) {
    // Basic validation
    if (!room.isVisited) return;

    for (int y = 0; y <= MAX_Y; ++y) {
        if (y < room.savedMapState.size()) {
            std::string line = room.savedMapState[y];

            for (int x = 0; x <= MAX_X; ++x) {
                if (x < line.size()) {
                    screen[y][x] = line[x];
                }
                else {
                    screen[y][x] = ' ';
                }
            }
        }
        else {
            // Fill extra rows with spaces if map is smaller than screen buffer
            for (int x = 0; x <= MAX_X; ++x) {
                screen[y][x] = ' ';
            }
        }
        // Always ensure null-termination
        screen[y][MAX_X + 1] = '\0';
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