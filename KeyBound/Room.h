#pragma once
#include <string>
#include <vector> 
#include "Point.h"
#include "Door.h"

struct Room {
    std::string mapFile;
    std::vector<Point> startPositions; 

    std::vector<std::string> savedMapState;

    bool isVisited = false;

    Room() = default;

    Room(const std::string& file, const std::vector<Point>& starts)
        : mapFile(file), startPositions(starts), isVisited(false)
    {
    }
};