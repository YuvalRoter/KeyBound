#pragma once
#include <string>
#include <vector>
#include "Point.h"

// Forward Declarations
class GameManger;
class Screen;

class Room {
private:
    // ===========================
    //       Private Data
    // ===========================

    std::string mapFile;                // The initial template file (e.g., "level1.txt")
    std::vector<std::string> savedMapState; // Saves the map state (removed keys, open doors) when leaving
    std::vector<Point> startPositions;  // Spawn points for P1 and P2
    bool dark = false;                  // Is this a "fog of war" level?
    bool isVisited = false;             // Have we been here before?

public:
    // ===========================
    //       Constructors
    // ===========================

    Room() = default;

    // Constructor used in GameManger::initRooms
    Room(const std::string& file, const std::vector<Point>& starts, bool isDark)
        : mapFile(file), startPositions(starts), dark(isDark), isVisited(false)
    {
    }

    // ===========================
    //       Friend Classes
    // ===========================
    // GameManger needs access to switch rooms and check 'dark' status.
    // Screen needs access to save/load the map characters into 'savedMapState'.
    friend class GameManger;
    friend class Screen;
};