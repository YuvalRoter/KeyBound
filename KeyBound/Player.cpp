#include <cstdlib>
#include <cctype>
#include <vector>
#include <iostream>

#include "Player.h"
#include "Direction.h"
#include "Door.h"
#include "utils.h"
#include "Screen.h"

// ===========================
//      Local Constants
// ===========================
namespace {
    constexpr int DEFAULT_SPEED = 1;
    constexpr char EMPTY_TILE = ' ';

    // Offsets for dropping items (Left, Right, Up, Down relative logic)
    // We will use logic to pick specific ones, but 2 is the max neighbors we check
    constexpr int MAX_NEIGHBOR_CHECKS = 2;
}

// Initialize static member
int Player::collectedKeys = 0;

// ===========================
//      Movement Logic
// ===========================
void Player::move(Door* doors, int maxDoors, int currentRoomIndex) {

    // 1. Visual Cleanup
    // If we are on a spring, draw the spring char so we don't "erase" it visually.
    // Otherwise, erase the player's old position.
    if (screen.isSpring(body))
        body.draw(Screen::SPRING);
    else
        body.draw(EMPTY_TILE);

    // 2. Handle Spring Physics (Launch State)
    if (isLaunched) {
        if (launchTimer > 0) {
            PlayerSpeed = launchSpeed; // Speed determined by compression
            launchTimer--;
        }
        else {
            // Launch finished, return to normal physics
            isLaunched = false;
            PlayerSpeed = DEFAULT_SPEED;
        }
    }

    // 3. Movement Loop
    // We move 'PlayerSpeed' tiles per frame (usually 1, unless launched)
    int stepsToTake = PlayerSpeed;

    for (int i = 0; i < stepsToTake; ++i) {

        // --- A. Determine Target Direction ---
        Direction moveDir;

        if (isLaunched) {
            // Mechanics: Forced movement in launch direction + Optional user control (Lateral)
            moveDir = launchDir;

            // Allow lateral movement (strafing) while in air
            // Logic: If launching Y (Up/Down), allow X input. If launching X (Left/Right), allow Y input.
            if (dir.getDirX() != 0 && launchDir.getDirY() != 0)
                moveDir = Direction(moveDir.getDirX() + dir.getDirX(), moveDir.getDirY());

            if (dir.getDirY() != 0 && launchDir.getDirX() != 0)
                moveDir = Direction(moveDir.getDirX(), moveDir.getDirY() + dir.getDirY());
        }
        else {
            // Standard walking
            moveDir = dir;
        }

        // --- B. Calculate Candidates ---
        Point next_pos = body + moveDir;

        // --- C. Collision & Interaction Checks ---

        // 1. Wall Collision
        if (screen.isWall(next_pos)) {
            // Special Case: Hitting a wall while compressing a spring triggers the launch
            if (springCompressedCount > 0) {
                startSpringLaunch();
                break; // Stop movement this frame
            }

            // Standard bonk
            if (!isLaunched) dir = Direction::directions[Direction::STAY];
            break; // Stop movement loop
        }

        // 2. Spring Interaction
        if (screen.isSpring(next_pos)) {
            if (!isLaunched) {
                // Compression phase: The longer we stay, the faster we launch
                springCompressedCount++;
            }
            // Move onto the spring (visual "squash" effect handled by draw logic)
            body = next_pos;
            continue; // Move successful, continue loop
        }
        else {
            // We stepped OFF a spring. Reset compression.
            if (!isLaunched && springCompressedCount > 0 && !screen.isSpring(next_pos)) {
                springCompressedCount = 0;
            }
        }

        // 3. Door Interaction
        if (screen.isDoor(next_pos)) {
            bool doorHandled = false;

            for (int k = 0; k < maxDoors; ++k) {
                // Optimization: Only check doors in this room
                if (doors[k].sourceRoomIndex == currentRoomIndex) {
                    if (doors[k].position == next_pos) {

                        // Case: Door Open -> Win/Exit
                        if (doors[k].isOpen) {
                            finishedLevel = true;
                            targetRoomIndex = doors[k].targetRoomIndex;
                            doorHandled = true;
                        }
                        // Case: Door Closed -> Try Keys
                        else if (tryToOpenDoor(doors[k].KeysToOpen)) {
                            doors[k].isOpen = true;
                            finishedLevel = true;
                            targetRoomIndex = doors[k].targetRoomIndex;
                            doorHandled = true;
                        }
                        // Case: Door Locked -> Block movement
                        else {
                            doorHandled = true; // Handled, but failed to enter
                        }
                        break; // Found the matching door object
                    }
                }
            }
            // Whether we opened it or hit it locked, we stop moving (it's solid)
            return;
        }

        // 4. Riddle Interaction
        if (screen.isRiddle(next_pos)) {
            body = next_pos; // Move onto the riddle tile

            // Clear the riddle from map so it doesn't trigger again immediately
            screen.setCell(body.getY(), body.getX(), EMPTY_TILE);

            // Save state and trigger Riddle Mode
            screen.saveBackup();
            screen.loadFromFileToMap("riddle1.txt"); 
            screen.draw();

            setInRiddle(true);
            return;
        }

        // 5. Torch Pickup
        if (screen.isTorch(next_pos)) {
            setTorch(true);

            // Remove torch from map
            screen.setCell(next_pos.getY(), next_pos.getX(), EMPTY_TILE);
            setHud(true); // Request HUD update
        }

        // 6. Key Pickup
        if (screen.isKey(next_pos)) {
            collectedKeys++;

 
            screen.setCell(next_pos.getY(), next_pos.getX(), EMPTY_TILE);

            setHud(true);
        }
        // 7. Victory Pickup
        if (screen.isWonChar(next_pos)){
            setWin(true);

        }
        // --- D. Commit Move ---
        body = next_pos;
    }

    // Render
    body.draw();

    // Reset Speed (if launch ended during this frame)
    if (!isLaunched && PlayerSpeed > DEFAULT_SPEED) {
        PlayerSpeed = DEFAULT_SPEED;
    }
}

// ===========================
//      Physics Logic
// ===========================

void Player::startSpringLaunch() {
    if (springCompressedCount == 0)
        return;

    // Speed is proportional to compression distance (time spent pressing)
    launchSpeed = springCompressedCount;

    // Duration is quadratic to give weight/impact to the launch
    launchTimer = springCompressedCount * springCompressedCount;

    // Reverse direction (Bounce)
    launchDir = Direction(-dir.getDirX(), -dir.getDirY());

    // Update flags
    isLaunched = true;
    springCompressedCount = 0;
}

void Player::keyPressed(char ch) {
    for (size_t i = 0; i < NUM_KEYS; ++i) {
        if (std::tolower(keys[i]) == std::tolower(ch)) {

            Direction newDir = Direction::directions[i];

            // Spring Logic: Trying to move AWAY or STOP while compressing triggers launch
            if (springCompressedCount > 0) {
                bool isStay = (newDir.getDirX() == 0 && newDir.getDirY() == 0);
                bool isChangeDir = (newDir.getDirX() != dir.getDirX() || newDir.getDirY() != dir.getDirY());

                if (isStay || isChangeDir) {
                    startSpringLaunch();
                    return;
                }
            }

            dir = newDir;
            return;
        }
    }
}

// ===========================
//      Inventory Logic
// ===========================

bool Player::tryToOpenDoor(int requiredKeys) {
    if (requiredKeys <= Player::collectedKeys) {
        collectedKeys -= requiredKeys;
        return true;
    }
    return false;
}

void Player::dropTorch() {
    if (!hasTorchFlag) {
        return;
    }

    int px = body.getX();
    int py = body.getY();
    int dx = dir.getDirX();
    int dy = dir.getDirY();

    // Calculate drop candidates (perpendicular to facing direction)
    // If facing Horizontal (X!=0), drop Up/Down.
    // If facing Vertical (Y!=0), drop Left/Right.
    struct Offset { int x, y; };
    Offset candidates[MAX_NEIGHBOR_CHECKS];

    if (dx != 0) { // Horizontal movement
        candidates[0] = { 0, -1 }; // Up
        candidates[1] = { 0, 1 };  // Down
    }
    else { // Vertical movement or Stay
        candidates[0] = { -1, 0 }; // Left
        candidates[1] = { 1, 0 };  // Right
    }

    // Find a valid spot
    int targetX = -1;
    int targetY = -1;

    for (int i = 0; i < MAX_NEIGHBOR_CHECKS; ++i) {
        int nx = px + candidates[i].x;
        int ny = py + candidates[i].y;

        // Boundary Check
        if (nx < 0 || nx > Screen::MAX_X || ny < 0 || ny > Screen::MAX_Y) {
            continue;
        }

        // Logic Check: Is the tile empty?
        if (screen.getCharAt(ny, nx) == EMPTY_TILE) {
            targetX = nx;
            targetY = ny;
            break;
        }
    }

    // Perform Drop
    if (targetX != -1) {
        screen.setCell(targetY, targetX, Screen::TORCH);

        Point torchPoint(targetX, targetY, Screen::TORCH);
        torchPoint.draw(); // Immediate visual update

        hasTorchFlag = false;
    }
}