#pragma once
#include "Point.h"

// Forward declarations:
// We tell the compiler these classes exist so we can befriend them.
class GameManger;
class Player;

class Door {
private:
    // ===========================
    //       Private Data
    // ===========================
    // These are now accessible ONLY to Door, GameManger, and Player.

    Point position;       // The specific (x,y) location
    int id = -1;          // Unique identifier
    int sourceRoomIndex = -1;
    int targetRoomIndex = -1;
    int KeysToOpen = 0;
    bool isOpen = false;

public:
    // ===========================
    //       Constructors
    // ===========================

    // Default constructor
    Door() = default;

    // Parameterized constructor for easy initialization in GameManager
    Door(Point pos, int id, int src, int dst, int cost, bool open)
        : position(pos), id(id), sourceRoomIndex(src), targetRoomIndex(dst), KeysToOpen(cost), isOpen(open)
    {
    }

    // ===========================
    //       Friend Classes
    // ===========================
    // This grants Player and GameManger full access to the private members above.
    friend class GameManger;
    friend class Player;
};