#pragma once

#include "Point.h"
#include "Screen.h"
#include "Direction.h"


class Door;

class Player {

    static constexpr size_t NUM_KEYS = 5;

    Point body;
    char keys[NUM_KEYS];
    Direction dir;
    Screen& screen;

    // Game State
    int PlayerSpeed = 1;
    bool won = false;
    bool inRiddleMode = false;
    bool finishedLevel = false;
    int targetRoomIndex = -1;
    bool waiting = false;
    int ourRoomIndex = 0;
    int pendingTargetRoom = -1;
    Point pendingSpawnPoint = Point(2, 3);

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
    static int collectedKeys;
    bool hasTorchFlag = false;
    bool HUD_changes = false;

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
    static int getCollectedKeys() {return collectedKeys; }
    bool isWaiting() const { return waiting; }
    int getPendingRoom() const { return pendingTargetRoom; }
    Point getPendingSpawn() const { return pendingSpawnPoint; }
    int getRoom() const { return ourRoomIndex; }
    bool getHUD() const { return HUD_changes; }

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
    void setInRiddle(bool state) { inRiddleMode = state;}
    void setWin(bool wins) { won = wins; }

    // Actions
    void addkey() { collectedKeys++; }
    void removeKeys(int keysToRemove) { collectedKeys -= keysToRemove; }
    void resetLevelData() {
        finishedLevel = false;
        targetRoomIndex = -1;
    }
 
    void move(Door* doors, int maxDoors, int currentRoomIndex);

    void startSpringLaunch();
    void keyPressed(char ch);
    bool tryToOpenDoor(int requiredKeys);
    void dropTorch();
};