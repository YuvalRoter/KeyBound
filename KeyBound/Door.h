#pragma once
#include "Point.h"
struct Door
{
    Point position;      // Where is it on the screen?
    int id;              // 1-9
    int targetRoomIndex; // Which room does it lead to?
    int KeysToOpen;
    bool isOpen = false;         // Is it unlocked?
    int sourceRoomIndex;  // Which room is this door sitting in?

};
    