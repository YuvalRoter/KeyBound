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
		jump();        // uses default NumberOfJumps = 3
		return;        // jump() already draws final position
	}	

	body.draw();
}

void Player::jump(int NumberOfJumps) {
	// IMPORTANT: do NOT erase here – move() already erased the starting cell
	Point old_head = body;  // starting point of the jump

	for (int i = 0; i < NumberOfJumps; ++i) {
		body.move();   // move one more step in current direction

		if (screen.isWall(body)) {
			// If we hit a wall at any point in the jump,
			// cancel whole jump and stay on the spring. 
			body = old_head;
			break;
		}

		if (screen.isWonChar(body)) {
			won = true;
			break;
		}

		// (optional) if we want chained springs
	}

	// draw final position (either reverted or after jump)
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
