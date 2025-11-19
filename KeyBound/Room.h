#pragma once
#include <string>
#include "Point.h"

struct Room {
    std::string mapFile;        // file name for this room
    Point       startPositions[2]; // starting position for the 2 players

    Room() = default;

    Room(const std::string& file,
        const Point& p1,
        const Point& p2)
        : mapFile(file)
    {
        startPositions[0] = p1;
        startPositions[1] = p2;
    }
};
