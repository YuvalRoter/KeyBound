#pragma once
#include "Point.h"
struct Door
{
    Point position;       // 1. Position
    int id = -1;          // 2. ID (Initialized to -1)
    int sourceRoomIndex = -1; // 3. Source (Where the door IS)
    int targetRoomIndex = -1; // 4. Target (Where the door LEADS)
    int KeysToOpen = 0;   // 5. Cost
    bool isOpen = false;  // 6. Status
};