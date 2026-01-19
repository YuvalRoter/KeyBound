#pragma once
#include <string>
#include "Point.h"

class Door {
public:
    int id = -1;             // The character on screen (e.g. '1', '2') - stored as int ID
    Point position;
    std::string targetFile;  // Name of the file this door leads to
    int keysRequired = 0;
    bool isOpen = false;

    Door() = default;
    Door(int id, Point pos, std::string target, int keys)
        : id(id), position(pos), targetFile(target), keysRequired(keys), isOpen(false) {
    }
};