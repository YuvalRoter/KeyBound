#pragma once

#include <string>
#include "Point.h"
#include <cctype>

// Forward declaration to reduce dependencies
class Room;

class Screen {
public:
    // ===========================
    //       Constants
    // ===========================
    static constexpr int MAX_X = 79;
    static constexpr int MAX_Y = 24;

    static constexpr int SIMON_WIDTH = 16;
    static constexpr int SIMON_HEIGHT = 8;
    static constexpr int SIMON_GAP_X = 4;
    static constexpr int SIMON_GAP_Y = 2;

    // Game Objects Characters
    static constexpr char WALL = '#';
    static constexpr char KEY = 'K';
    static constexpr char RIDDEL = '?';
    static constexpr char DOOR = 'D';
    static constexpr char SPRING = '+';
    static constexpr char BOMB = 'B';
    static constexpr char PLAYER1 = '@';
    static constexpr char PLAYER2 = '&';
    static constexpr char WONCHAR = '%';
    static constexpr char HEARTCHAR = '$';
    static constexpr char TORCH = 'T';
    static constexpr char OBSTACLE = 'O';
   

    // Color Codes (Console Specific)
    enum Color {
        Black = 0,
        Blue = 1,
        Green = 2,
        Cyan = 3,
        Red = 4,
        Magenta = 5,
        Brown = 6,
        LightGray = 7,
        DarkGray = 8,
        LightBlue = 9,
        LightGreen = 10,
        LightCyan = 11,
        LightRed = 12,
        LightMagenta = 13,
        Yellow = 14,
        White = 15,
        BLANK = -1
    };

    // Extended ASCII Block Characters
    enum BlockType {
        LightBlock = 176,
        MediumBlock = 177,
        DarkBlock = 178,
        FullBlock = 219
    };

    // ===========================
    //       Constructor
    // ===========================
    Screen() {
        // Initialize with null terminator
        screen[0][0] = '\0';
        backup[0][0] = '\0';
        hasBackup = false;
    }

private:
    // ===========================
    //       Private Data
    // ===========================
    char screen[MAX_Y + 1][MAX_X + 2];
    char backup[MAX_Y + 1][MAX_X + 2];
    bool hasBackup = false;

    // Helper to get char at specific Point
    char charAt(const Point& p) const {
        return screen[p.getY()][p.getX()];
    }

public:
    // ===========================
    //       Map Management
    // ===========================

    // Saves current screen char array into the Room object.
    void saveScreenToRoom(Room& room) const;

    // Overwrites screen char array with data from the Room object.
    void loadScreenFromRoom(const Room& room);

    // Creates a memory backup of the current screen state.
    void saveBackup();

    // Restores the screen from the memory backup.
    void restoreBackup();

    // Loads a raw map file (.txt) into the screen array.
    bool loadFromFileToMap(const std::string& filename);

    // ===========================
    //       Setters/Getters
    // ===========================

    void setCell(int y, int x, char c) {
        // Boundary check is recommended here in a real project
        screen[y][x] = c;
    }

    char getCharAt(int y, int x) const {
        return screen[y][x];
    }

    // ===========================
    //       Type Checkers
    // ===========================

    bool isWall(const Point& p) const { return charAt(p) == WALL; }
    bool isRiddle(const Point& p) const { return charAt(p) == RIDDEL; }
    bool isWonChar(const Point& p) const { return charAt(p) == WONCHAR; }
    bool isSpring(const Point& p) const { return charAt(p) == SPRING; }
    bool isKey(const Point& p) const { return charAt(p) == KEY; }
    bool isTorch(const Point& p) const { return charAt(p) == TORCH; }
    // Checks if the character is a digit (representing a door ID)
    bool isDoor(const Point& p) const { return isdigit(charAt(p)); }
	bool isObstacle(const Point& p) const { return charAt(p) == OBSTACLE; }
    // ===========================
    //       Rendering
    // ===========================

    // Renders the entire screen array to the console
    void draw() const;

    // Renders a filled rectangle for the Simon Says game
    void drawSimonSquare(int left, int top, char filler) const;

    // Orchestrates the Simon Says drawing logic
    // param litIndex: The index (0-3) of the square to highlight, or -1 for none
    void drawSimon(int litIndex) const;
};