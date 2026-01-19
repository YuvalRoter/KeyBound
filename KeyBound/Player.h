#pragma once
#include "Point.h"
#include "Direction.h"
#include "Screen.h"
#include "Door.h"
#include <vector>

class Player {
public:
    using Controls = std::vector<char>;

private:
    Point position;
    Direction dir;
    Controls keys;
    Screen& screen;
    char texture;

    bool hasTorchFlag = false;
    bool hasBombFlag = false;
    bool finishedLevel = false;
    bool won = false;
    bool inRiddleState = false;
    bool hudUpdateNeeded = false;
    bool trapState = false;
    Point trapLocation;

public:
    static int collectedKeys;   
    static int AmountOfSwitches;  

    Player(Point pos, Direction d, Controls k, Screen& scr);
    Player() = default;

    void move(Door* doors, int maxDoors, int currentRoom, Player* otherPlayer, bool isLight);
    void keyPressed(char key);
    Point dropActiveItem(char& type);

    Point getPoint() const { return position; }
    void setPosition(Point p) { position = p; }
    int getX() const { return position.getX(); }
    int getY() const { return position.getY(); }
    char getChar() const { return texture; }

    void setDirection(Direction d) { dir = d; }

    bool hasTorch() const { return hasTorchFlag; }
    void setTorch(bool t) { hasTorchFlag = t; }

    bool hasBomb() const { return hasBombFlag; }
    void setBomb(bool b) { hasBombFlag = b; }

    bool isFinished() const { return finishedLevel; }
    void setFinished(bool f) { finishedLevel = f; }

    bool hasWon() const { return won; }
    void setWin(bool w) { won = w; }

    bool inRiddle() const { return inRiddleState; }
    void setInRiddle(bool r) { inRiddleState = r; }

    bool getHUD() const { return hudUpdateNeeded; }
    void setHud(bool h) { hudUpdateNeeded = h; }

    bool getTrapState() const { return trapState; }
    void setTrapState(bool s) { trapState = s; }
    Point getTrapLocation() const { return trapLocation; }

    char getstaybutton() { return keys[4]; }

    void resetLevelData();
    void removeKeys(int amount);

    // פונקציות חדשות שחסרות לך:
    static void resetKeys();
    static int getCollectedKeys() { return collectedKeys; }

    int ourRoomIndex = -1;
    int targetRoomIndex = -1;
};