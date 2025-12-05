#include <cstdlib>
#include <cctype>
#include "Player.h"
#include "Direction.h"

void Player::move() {


	body.draw(' ');// erase old position
    int current_speed = PlayerSpeed;

    for (int i = 0; i < current_speed; ++i) {
    Point next_pos = body + dir;

	if (screen.isWall(next_pos)) {
        dir = Direction::directions[Direction::STAY];
        break;
		
	}
    if (screen.isWonChar(next_pos)) {
        body = next_pos;
        won = true;
        break;
    }

    if (screen.isDoor(next_pos)) {
        finishedLevel = true; // Mark as done
        body.draw(' ');       // Erase character (visualize entering the door)
        return;               // Stop moving immediately
    }
   
    if (screen.isSpring(next_pos)) {
        // Logic: Land on spring, reverse, and charge speed for NEXT turn
        body = next_pos;      
        dir = dir * -1;       // Reverse direction
        PlayerSpeed += 2;      // Add 2 speed
        break;                // Stop moving for this turn
    }

    // --- CHECK RIDDLE ---
    if (screen.isRiddle(next_pos)) {
        body = next_pos;
        screen.setCell(body.getY(), body.getX(), ' ');
        screen.saveBackup();
        screen.loadFromFileToMap("riddle1.txt");
        screen.draw();
        Player::Riddle = true;
        return; // Exit function completely
    }


    body = next_pos;
    }

    
    body.draw();
    //reset speed
    if (PlayerSpeed > 1 && !screen.isSpring(body)) {
        PlayerSpeed = 1;
    }
}
void Player::keyPressed(char ch) {
    for (size_t i = 0; i < NUM_KEYS; ++i) {
        if (std::tolower(keys[i]) == std::tolower(ch)) {

        
            dir = Direction::directions[i];

            return; // We found the key, exit 
        }
    }
}
