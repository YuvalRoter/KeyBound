#include <cstdlib>
#include <cctype>
#include "Player.h"
#include "Direction.h"

void Player::move() {
	body.draw(' ');// erase old position


	Point old_head = body;//save old position in case we hit a wall
	body.move();// move to new position
	if (screen.isWall(body)) {

		body = old_head;
	}
	else if (screen.isWonChar(body)) {
		won = true;
	}	
	else if (screen.isSpring(body)) {
		body.changeDir(body.getDir()*-1);
		jump();        // uses default NumberOfJumps = 3
		return;        // jump() already draws final position
	}	
	else if (screen.isRiddle(body)) {

		// 1. Remove the ? from the map so riddle doesn't repeat
		screen.setCell(body.getY(), body.getX(), ' ');   
		screen.saveBackup();

		// 2. Load the riddle screen
		screen.loadFromFileToMap("riddle1.txt");
		screen.draw();

		Player::Riddle = true;
		return;
	}

	body.draw();
}
void Player::jump(int NumberOfJumps) {
    // starting point: we are already standing on the spring
    Point start = body;

    for (int i = 0; i < NumberOfJumps; ++i) {
        Point prev = body;    // remember previous position
        body.move();          // move one step in current direction

        // If we hit a WALL, stand just before it 
        if (screen.isWall(body)) {
            body = prev;     
            break;
        }

        // If we land on the WIN tile, stop there and mark win
        if (screen.isWonChar(body)) {
            won = true;
            break;
        }

        // If we land on a RIDDLE during the jump – trigger it immediately
        if (screen.isRiddle(body)) {
            screen.setCell(body.getY(), body.getX(), ' ');
            screen.saveBackup();
            screen.loadFromFileToMap("riddle1.txt");
            screen.draw();

            Player::Riddle = true;
            body.draw();
            return;
        }

        // If we land on ANOTHER SPRING:
        // stop here; next game tick `move()` will see the spring and
        // start a new jump automatically. This makes spring–spring chains work.
        if (screen.isSpring(body)) {
            break;
        }

        
    }

    // draw final position (either before a wall, or on some special char)
    body.draw();
}


void Player::keyPressed(char ch) {
	size_t index = 0;
	for (char key : keys) {
		if (std::tolower(key) == std::tolower(ch)) {
			body.changeDir(Direction::directions[index]);
			break;
		}
		++index;
	}
}
