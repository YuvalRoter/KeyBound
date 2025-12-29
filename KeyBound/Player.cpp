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

    bool inBounds(int x, int y) {
        return x >= 0 && x <= Screen::MAX_X && y >= 0 && y <= Screen::MAX_Y;
    }

    // Collect all connected obstacle tiles (4-direction only)
    // Collect only the obstacle "slice" along the push axis:
// - pushing LEFT/RIGHT -> collect contiguous obstacles in the same ROW
// - pushing UP/DOWN    -> collect contiguous obstacles in the same COLUMN
// Collect only the obstacle "slice" along the push axis:
// - pushing LEFT/RIGHT -> collect contiguous obstacles in the same ROW
// - pushing UP/DOWN    -> collect contiguous obstacles in the same COLUMN
// Collect only the obstacle "slice" along the push axis:
// - pushing LEFT/RIGHT -> collect contiguous obstacles in the same ROW
// - pushing UP/DOWN    -> collect contiguous obstacles in the same COLUMN
// Collect only the obstacle "slice" along the push axis:
// - pushing LEFT/RIGHT -> contiguous obstacles in the same ROW
// - pushing UP/DOWN    -> contiguous obstacles in the same COLUMN
    
    int sgn(int v) { return (v > 0) - (v < 0); }

    // Collect only the obstacle "slice" along the push axis, based on actual screen delta (dx,dy)
    std::vector<Point> collectObstacleCells(Screen& screen, const Point& start, int dx, int dy) {
        std::vector<Point> cells;

        if (!inBounds(start.getX(), start.getY())) return cells;
        if (screen.getCharAt(start.getY(), start.getX()) != Screen::OBSTACLE) return cells;

        dx = sgn(dx);
        dy = sgn(dy);

        // No diagonal pushing
        if (dx != 0 && dy != 0) return cells;
        if (dx == 0 && dy == 0) return cells;

        // Move to the beginning of the contiguous slice in the negative axis direction
        int x = start.getX();
        int y = start.getY();
        while (inBounds(x - dx, y - dy) &&
            screen.getCharAt(y - dy, x - dx) == Screen::OBSTACLE) {
            x -= dx;
            y -= dy;
        }

        // Collect contiguous cells forward on that axis
        while (inBounds(x, y) && screen.getCharAt(y, x) == Screen::OBSTACLE) {
            cells.emplace_back(x, y);
            x += dx;
            y += dy;
        }

        return cells;
    }

    bool canPushObstacle(Screen& screen, const std::vector<Point>& cells, int dx, int dy) {
        dx = sgn(dx);
        dy = sgn(dy);

        bool inGroup[Screen::MAX_Y + 1][Screen::MAX_X + 1] = { false };
        for (const auto& p : cells) {
            inGroup[p.getY()][p.getX()] = true;
        }

        for (const auto& p : cells) {
            int nx = p.getX() + dx;
            int ny = p.getY() + dy;

            if (!inBounds(nx, ny)) return false;

            // Moving into itself is fine
            if (inGroup[ny][nx]) continue;

            // Only push into empty space
            if (screen.getCharAt(ny, nx) != EMPTY_TILE) return false;
        }

        return true;
    }

    void pushObstacleOneStep(Screen& screen, const std::vector<Point>& cells, int dx, int dy, bool redrawNow) {
        dx = sgn(dx);
        dy = sgn(dy);

        // Clear old cells
        for (const auto& p : cells) {
            screen.setCell(p.getY(), p.getX(), EMPTY_TILE);
            if (redrawNow) Point(p.getX(), p.getY()).draw(EMPTY_TILE);
        }

        // Place new cells
        for (const auto& p : cells) {
            int nx = p.getX() + dx;
            int ny = p.getY() + dy;
            screen.setCell(ny, nx, Screen::OBSTACLE);
            if (redrawNow) Point(nx, ny).draw(Screen::OBSTACLE);
        }
    }

}

// Initialize static member
int Player::collectedKeys = 0;
int Player::AmountOfSwitches = 0;

// ===========================
//      Movement Logic
// ===========================
void Player::move(Door* doors, int maxDoors, int currentRoomIndex, Player* otherPlayer, bool redrawMapNow) {

    // ===========================
     // 1. Visual Cleanup
     // ===========================
     // We look at the map to see what we are currently standing on.
     // This handles Springs, Switches, and Empty Tiles automatically.
    char objectUnderPlayer = screen.getCharAt(body.getY(), body.getX());

    // If the map data says "Player" is here, we treat it as empty floor underneath.
    if (objectUnderPlayer == Screen::PLAYER1 || objectUnderPlayer == Screen::PLAYER2) {
        body.draw(EMPTY_TILE);
    }
    else {
        // Restore the object (Switch ON/OFF, Spring, etc.)
        body.draw(objectUnderPlayer);
    }

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

        // 1.5 Player Collision (treat the other player as solid)
        if (otherPlayer != nullptr && !otherPlayer->isFinished() && next_pos == otherPlayer->body) {
            if (!isLaunched) dir = Direction::directions[Direction::STAY];
            break;
        }

        // 1.6 Obstacle Interaction (pushable blocks)
        if (screen.isObstacle(next_pos)) {

            // Compute push delta in REAL screen coordinates
            int pushDx = next_pos.getX() - body.getX();
            int pushDy = next_pos.getY() - body.getY();
            pushDx = sgn(pushDx);
            pushDy = sgn(pushDy);

            // No diagonal pushing
            if (pushDx != 0 && pushDy != 0) {
                if (!isLaunched) dir = Direction::directions[Direction::STAY];
                break;
            }

            // Base force
            int totalForce = isLaunched ? PlayerSpeed : 1;

            // Add force from the other player if he is directly behind us and moving same direction
            if (otherPlayer != nullptr) {

                // Compute the other player's effective movement direction (same logic as your move)
                Direction otherMoveDir;
                if (otherPlayer->isLaunched) {
                    otherMoveDir = otherPlayer->launchDir;

                    if (otherPlayer->dir.getDirX() != 0 && otherPlayer->launchDir.getDirY() != 0)
                        otherMoveDir = Direction(otherMoveDir.getDirX() + otherPlayer->dir.getDirX(), otherMoveDir.getDirY());

                    if (otherPlayer->dir.getDirY() != 0 && otherPlayer->launchDir.getDirX() != 0)
                        otherMoveDir = Direction(otherMoveDir.getDirX(), otherMoveDir.getDirY() + otherPlayer->dir.getDirY());
                }
                else {
                    otherMoveDir = otherPlayer->dir;
                }

                // Convert other player's intent into REAL screen delta
                Point otherNext = otherPlayer->body + otherMoveDir;
                int otherDx = sgn(otherNext.getX() - otherPlayer->body.getX());
                int otherDy = sgn(otherNext.getY() - otherPlayer->body.getY());

                bool sameDir = (otherDx == pushDx && otherDy == pushDy);

                if (sameDir) {
                    Point behind(body.getX() - pushDx, body.getY() - pushDy);
                    if (otherPlayer->body == behind) {
                        int otherForce = otherPlayer->isLaunched ? otherPlayer->PlayerSpeed : 1;
                        totalForce += otherForce;
                    }
                }
            }

            // Collect only the slice along the push axis
            std::vector<Point> obstacleCells = collectObstacleCells(screen, next_pos, pushDx, pushDy);
            int requiredForce = (int)obstacleCells.size();

            if (requiredForce > 0 &&
                totalForce >= requiredForce &&
                canPushObstacle(screen, obstacleCells, pushDx, pushDy)) {

                pushObstacleOneStep(screen, obstacleCells, pushDx, pushDy, redrawMapNow);

                // Move player into the obstacle's previous tile
                body = next_pos;
                continue;
            }
            else {
                if (!isLaunched) dir = Direction::directions[Direction::STAY];
                break;
            }
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
            if (!isLaunched && springCompressedCount > 0) {
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

                        // Case: Door Open -> Exit
                        if (doors[k].isOpen) {
                            finishedLevel = true;
                            targetRoomIndex = doors[k].targetRoomIndex;
                            doorHandled = true;
                        }
                        // Case: Switchs on?   
                        // 8 - indicator for switch door
                        if (doors[k].KeysToOpen == SWITCH_ID)
                        {
                            // Only open if the switch count matches the target
                            if (AmountOfSwitches >= SwitchsToTurn)
                            {
                                doors[k].isOpen = true;
                                finishedLevel = true;
                                targetRoomIndex = doors[k].targetRoomIndex;
                                doorHandled = true;
                            }
                            else
                            {
                                // Switch count not met; door remains closed (collision happens)
                                doorHandled = true;
                            }
                        }
                        // If not a switch door, try standard key opening
                        else if (tryToOpenDoor(doors[k].KeysToOpen))
                        {
                            doors[k].isOpen = true;
                            finishedLevel = true;
                            targetRoomIndex = doors[k].targetRoomIndex;
                            doorHandled = true;
                        }
                        // Door is locked and we have no keys
                        else
                        {
                            doorHandled = true;
                        }
                        break;
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

        // 5. Item Pickup Logic

        char item = screen.getCharAt(next_pos.getY(), next_pos.getX());

        if (item == Screen::TORCH) {
            if (!hasBombFlag && !hasTorchFlag) { // Restriction: Cannot hold more then one item
                setTorch(true);
                screen.setCell(next_pos.getY(), next_pos.getX(), ' ');
                setHud(true);
            }
        }
        else if (item == Screen::BOMB) {
            if (!hasTorchFlag && !hasBombFlag) { // Restriction: Cannot hold more then one item
                setBomb(true);
                screen.setCell(next_pos.getY(), next_pos.getX(), ' ');
                setHud(true);
            }
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
        // 8. Switch Logic
        if (screen.isSwitchOff(next_pos)) {

            screen.setCell(next_pos.getY(), next_pos.getX(), Screen::SWITCH_ON);
            AmountOfSwitches++;

        }
        else if (screen.isSwitchOn(next_pos)) {
            screen.setCell(next_pos.getY(), next_pos.getX(), Screen::SWITCH_OFF);
            AmountOfSwitches--;
        }
        // 9. Trap Logic
        if (screen.isTrap(next_pos)){
            screen.setCell(next_pos.getY(), next_pos.getX(), Screen::BOMB_ACTIVE);
            trapLocation = next_pos;
            TrapActive = true;
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

Point Player::dropActiveItem(char& droppedType) {
    droppedType = ' ';

    // 1. Check Inventory
    if (hasTorchFlag) droppedType = Screen::TORCH;
    else if (hasBombFlag) droppedType = Screen::BOMB; // Ensure Screen::BOMB is defined as 'B'
    else return Point(-1, -1); // Holding nothing

    // 2. Find a free spot
    int px = body.getX();
    int py = body.getY();
    struct Offset { int x, y; };
    Offset candidates[] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} }; // Check Up, Down, Left, Right

    int targetX = -1;
    int targetY = -1;

    for (const auto& off : candidates) {
        int nx = px + off.x;
        int ny = py + off.y;

        // Ensure we are inside the screen
        if (nx < 0 || nx > Screen::MAX_X || ny < 0 || ny > Screen::MAX_Y) continue;

        // Check if the tile is empty space
        // Make sure your floor tiles are actually spaces ' ' and not something else
        if (screen.getCharAt(ny, nx) == ' ') {
            targetX = nx;
            targetY = ny;
            break; // Found a spot!
        }
    }

    // 3. Drop the item
    if (targetX != -1) {
        screen.setCell(targetY, targetX, droppedType);
        Point(targetX, targetY, droppedType).draw();

        if (droppedType == Screen::TORCH) hasTorchFlag = false;
        if (droppedType == Screen::BOMB) hasBombFlag = false;

        setHud(true);
        return Point(targetX, targetY, droppedType);
    }

    // If we reach here, we are surrounded by walls/objects and cannot drop.
    return Point(-1, -1);
}