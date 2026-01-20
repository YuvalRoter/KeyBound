#pragma once

#include "Point.h"
#include "Screen.h"
#include "Door.h"
#include "Direction.h"
#include <vector>



class Door;

class Player {
    friend class GameManger;
    static constexpr size_t NUM_KEYS = 5;

    static constexpr size_t SwitchsToTurn = 3;

    static constexpr size_t SWITCH_ID = 8;

    Point body;
    char keys[NUM_KEYS];
    Direction dir;
    Screen& screen;

    // Game State
    int PlayerSpeed = 1;
    static int AmountOfSwitches;
    bool won = false;
    bool inRiddleMode = false;
    bool finishedLevel = false;
    int targetRoomIndex = -1;
    bool waiting = false;
    int ourRoomIndex = 0;
    int pendingTargetRoom = -1;
    Point pendingSpawnPoint = Point(2, 3);

    bool TrapActive = false;
    Point trapLocation;

    // Spring / Launch Variables
    int springCompressed = 0;
    bool springCompressing = false;
    Direction springCompressionDir = Direction::directions[Direction::STAY];
    Direction springBoostDir = Direction::directions[Direction::STAY];
    int springBoostSpeed = 1;
    int springBoostTimer = 0;
    int springCompressedCount = 0;

    // Launch State
    bool isLaunched = false;
    int launchTimer = 0;
    int launchSpeed = 0;
    Direction launchDir = { 0,0 };

    // Inventory
    std::vector<char> heldKeys;   // each key is a real item (specific instance)
    bool hasTorchFlag = false;
    bool HUD_changes = false;
    bool hasBombFlag = false;

public:
    struct Controls {
        char up, right, stay, left, pause;
    };

    Player() = default;

    Player(const Point& start_point, const Direction& start_dir, const Player::Controls& controls, Screen& theScreen, int speed = 1)
        : body(start_point), dir(start_dir), screen(theScreen), PlayerSpeed(speed)
    {
        keys[0] = controls.up;
        keys[1] = controls.right;
        keys[2] = controls.stay;
        keys[3] = controls.left;
        keys[4] = controls.pause;
    }

    // Getters
    char getChar() const { return body.getChar(); }
    char getstaybutton() const { return keys[4]; }
    bool hasWon() const { return won; }
    bool inRiddle() const { return inRiddleMode; }
    Point getPoint() const { return body; }
    bool isFinished() const { return finishedLevel; }
    int getX() const { return body.getX(); }
    int getY() const { return body.getY(); }
    int getTargetRoom() const { return targetRoomIndex; }
    bool hasTorch() const { return hasTorchFlag; }
    int getKeyCount() const { return (int)heldKeys.size(); }
    bool isWaiting() const { return waiting; }
    int getPendingRoom() const { return pendingTargetRoom; }
    Point getPendingSpawn() const { return pendingSpawnPoint; }
    int getRoom() const { return ourRoomIndex; }
    bool getHUD() const { return HUD_changes; }
    bool hasBomb() const { return hasBombFlag; }
    bool getTrapState() const { return TrapActive; }
    Point getTrapLocation() const { return trapLocation; }


    // Setters
    void setFinished(bool state) { finishedLevel = state; }
    void setPosition(const Point& p) { body = p; }
    void setDirection(Direction d) { dir = d; }
    void setTorch(bool v) { hasTorchFlag = v; }
    void setWaiting(bool w) { waiting = w; }
    void setPendingMove(int room, Point spawn) {
        pendingTargetRoom = room;
        pendingSpawnPoint = spawn;
    }
    void setRoom(int room) { ourRoomIndex = room; }
    void setHud(bool b) { HUD_changes = b; }
    void setInRiddle(bool state) { inRiddleMode = state; }
    void setWin(bool wins) { won = wins; }
    void setBomb(bool v) { hasBombFlag = v; }
    void setTrapState(bool s) { TrapActive = s; }

    // Actions
    void addKey() { heldKeys.push_back(Screen::KEY); }
    void clearKeys() { heldKeys.clear(); }
    void setKeyCount(int n) {
        if (n < 0) n = 0;
        heldKeys.assign((size_t)n, Screen::KEY);
    }

    void resetLevelData() {
        finishedLevel = false;
        targetRoomIndex = -1;
    }
    Point dropActiveItem(char& droppedType, bool isSilent);
    void move(std::vector<Door>& doors, int currentRoomIndex, Player* otherPlayer, bool redrawMapNow, bool isSilent);

    void startSpringLaunch();
    void keyPressed(char ch);
    bool tryToOpenDoor(int requiredKeys);
};