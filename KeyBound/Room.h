#pragma once
#include <string>
#include <vector>
#include <map>
#include "Point.h"
#include "Door.h"

class Room {
public:
    std::string filename;
    std::vector<std::string> mapData;
    std::vector<std::string> savedMapState;
    std::map<int, Door> doors; // Map door ID to Door object

    Point p1Start = { -1, -1 };
    Point p2Start = { -1, -1 };
    Point legendLoc = { 0, 0 };

    bool isDark = false;
    bool isVisited = false;

    Room() = default;
};