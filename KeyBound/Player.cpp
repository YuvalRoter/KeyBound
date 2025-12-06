#include <cstdlib>
#include <cctype>
#include "Player.h"
#include "Direction.h"

void Player::move() {


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

        if (screen.isDoor(next_pos)) {
            finishedLevel = true;
            body.draw(' ');
            return;
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
    launchDir = dir * -1;

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

