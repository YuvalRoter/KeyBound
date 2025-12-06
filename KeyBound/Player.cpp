#include <cstdlib>
#include <cctype>
#include "Player.h"
#include "Direction.h"
#include <vector>
#include "Door.h"
#include "utils.h"

void Player::move(Door* doors, int maxDoors, int currentRoomIndex) {

    if (screen.isSpring(body)) 
        body.draw(SPRING); 
    else
	    body.draw(' ');// erase old position

    if (isLaunched) {
        if (launchTimer > 0) {
            PlayerSpeed = launchSpeed;
            launchTimer--;
            // Force the direction, but allow lateral movement 
        }
        else {
            isLaunched = false;
            PlayerSpeed = 1; // Reset to normal speed
        }
    }

    int stepsToTake = PlayerSpeed;
    for (int i = 0; i < stepsToTake; ++i) {

        // A. Determine Target Direction
        Direction moveDir;

        if (isLaunched) {
            // If launched, we MUST move in launchDir.
            // But we can add lateral (sideways) movement from user input keys.
            moveDir = launchDir;

            // Check if user is pressing a key perpendicular to launch
            // (e.g. Launching UP, user presses RIGHT -> Diagonal move)
            if (dir.getDirX() != 0 && launchDir.getDirY() != 0) moveDir = Direction(moveDir.getDirX() + dir.getDirX(), moveDir.getDirY());
            if (dir.getDirY() != 0 && launchDir.getDirX() != 0) moveDir = Direction(moveDir.getDirX(), moveDir.getDirY() + dir.getDirY());
        }
        else {
            // Normal movement
            moveDir = dir;
        }

        // B. Calculate Next Position
        Point next_pos = body + moveDir;

        // --- COLLISION CHECKS ---

        // Wall Collision
        if (screen.isWall(next_pos)) {
            if (springCompressedCount > 0) {
               
                startSpringLaunch();
                break; // Break this frame to allow direction reset
            }
            // Standard wall hit
            if (!isLaunched) dir = Direction::directions[Direction::STAY];
            break;
        }

        // Spring Logic (Compression Phase)
        if (screen.isSpring(next_pos)) {
            // If we are NOT launched, we are compressing
            if (!isLaunched) {
                springCompressedCount++;
                // "Collapse" visual: We move onto it, effectively hiding the '+'
                // (The previous '+' is erased by body.draw(' ') at start of loop)
            }
            // Move player onto the spring
            body = next_pos;
            continue; // Continue to next step in speed loop
        }
        else {
            // We stepped OFF a spring or onto normal ground. 
            // If we were compressing but didn't hit a wall, we just walked off the spring.
            // Reset compression if we wander off.
            if (!isLaunched && springCompressedCount > 0 && !screen.isSpring(next_pos)) {
                springCompressedCount = 0;
            }
        }
        // CHECK FOR DOORS
        if (screen.isDoor(next_pos)) {

            // Loop through the Global List
            for (int i = 0; i < maxDoors; ++i) {

                // FILTER: Only look at doors that are in THIS room
                if (doors[i].sourceRoomIndex == currentRoomIndex) {

                    // CHECK: Is this the specific door we stepped on?
                    if (doors[i].position == next_pos) {

                        // CASE A: Already Open (e.g., Player 2 follows Player 1)
                        if (doors[i].isOpen) {
                            finishedLevel = true; // Level Done

                  
                            targetRoomIndex = doors[i].targetRoomIndex;
                            return;
                        }

                        // CASE B: Closed -> Try to open
                        else if (tryToOpenDoor(doors[i].KeysToOpen)) {
                            doors[i].isOpen = true; // Mark as permanently open
                            finishedLevel = true;   // Level Done

                            // [FIX 2] SAVE THE DESTINATION
                            targetRoomIndex = doors[i].targetRoomIndex;

                           
                            return;
                        }

                        // CASE C: Locked -> Not enough keys
                        else {
                           
                         
                            return;
                        }
                    }
                }
            }
            return; // We hit a door (locked or not), so stop moving
        }

        // Riddle Logic
        if (screen.isRiddle(next_pos)) {
            body = next_pos;
           
            screen.setCell(body.getY(), body.getX(), ' ');
            screen.saveBackup();
            screen.loadFromFileToMap("riddle1.txt");
            screen.draw();
            Player::Riddle = true;
            return;
        }
        if (screen.isTorch(next_pos)) {
            setTorch(true); // now this player has a torch
            screen.setCell(next_pos.getY(), next_pos.getX(), ' ');
        }

        if (screen.isKey(next_pos)) {
            collectedKeys++;
            screen.setCell(next_pos.getY(), next_pos.getX(), ' ');
               
            gotoxy(30, 0);
            std::cout << collectedKeys;
        }

        //Commit Move
        body = next_pos;

    }

    // Restore Visuals
    body.draw();

    // Reset speed if we are done with the launch
    if (!isLaunched && PlayerSpeed > 1) {
        PlayerSpeed = 1;
    }
}

void Player::startSpringLaunch() {
    if (springCompressedCount == 0) 
        return;

    // 1. Calculate Mechanics
    launchSpeed = springCompressedCount;              // Speed = N
    launchTimer = springCompressedCount * springCompressedCount; // Time = N^2

    // 2. Direction is opposite of current facing (bounce back)
    launchDir = Direction(-dir.getDirX(), -dir.getDirY());

    // 3. Set State
    isLaunched = true;
    springCompressedCount = 0; // Reset compression
}


void Player::keyPressed(char ch) {
    for (size_t i = 0; i < NUM_KEYS; ++i) {
        if (std::tolower(keys[i]) == std::tolower(ch)) {

            Direction newDir = Direction::directions[i];

            // --- SPRING TRIGGER CHECK ---
            // If we are currently compressing a spring (on top of it)
            if (springCompressedCount > 0) {
                // If user presses STAY or tries to reverse/turn
                if (newDir.getDirX() == 0 && newDir.getDirY() == 0) {
                    startSpringLaunch();
                    return;
                }
                // Check if direction changed (simple check against current dir)
                if (newDir.getDirX() != dir.getDirX() || newDir.getDirY() != dir.getDirY()) {
                    startSpringLaunch();
                    return;
                }
            }

            dir = newDir;
            return;
        }
    }
}

bool Player::tryToOpenDoor(int requiredKeys) {
    if (requiredKeys <= Player::collectedKeys) {
        collectedKeys -= requiredKeys;
        gotoxy(30, 0);
        std::cout << collectedKeys;
        return true;
    }
    return false;
}

void Player::dropTorch() {
    //No torch ? nothing to drop
    if (!hasTorchFlag) {
        return;
    }

    int px = body.getX();
    int py = body.getY();

    //Decide side directions based on current facing dir
    int dx = dir.getDirX();
    int dy = dir.getDirY();

    int offsets[2][2];

    if (dx != 0 && dy == 0) {
        //Moving horizontally (left/right):
        //drop to the SIDE: up or down (not along the path)
        offsets[0][0] = 0; offsets[0][1] = -1; // up
        offsets[1][0] = 0; offsets[1][1] = 1; // down
    }
    else if (dx == 0 && dy != 0) {
        //Moving vertically (up/down):
        //drop to the SIDE: left or right (not along the path)
        offsets[0][0] = -1; offsets[0][1] = 0; // left
        offsets[1][0] = 1; offsets[1][1] = 0; // right
    }
    else {
        //STAY (0,0) or undefined ? choose some default (left/right)
        offsets[0][0] = -1; offsets[0][1] = 0; // left
        offsets[1][0] = 1; offsets[1][1] = 0; // right
    }

    int tx = -1;
    int ty = -1;

    //Find first empty side tile
    for (int i = 0; i < 2; ++i) {
        int nx = px + offsets[i][0];
        int ny = py + offsets[i][1];

        if (nx < 0 || nx > Screen::MAX_X ||
            ny < 0 || ny > Screen::MAX_Y) {
            continue;
        }

        char here = screen.getCharAt(ny, nx);
        if (here == ' ') { //empty floor on the map
            tx = nx;
            ty = ny;
            break;
        }
    }

    //No free side tile ? cannot drop
    if (tx == -1) {
        return;
    }

    //Write torch into the map
    screen.setCell(ty, tx, TORCH);

    //Draw the torch directly so it appears immediately
    Point torchPoint(tx, ty, TORCH);
    torchPoint.draw();

    //Player no longer has the torch
    hasTorchFlag = false;
}