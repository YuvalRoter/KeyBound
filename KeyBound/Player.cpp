#include <cstdlib>
#include <cctype>
#include "Player.h"
#include "Direction.h"

void Player::move() {
	body[SIZE - 1].draw(' '); // erase the tail from screen
	for (size_t index = SIZE - 1; index > 0; --index) {
		body[index] = body[index - 1];
	}
	Point old_head = body[0];
	body[0].move();
	if (screen.isWall(body[0])) {

		body[0] = old_head;
	}
	else if (screen.isWonChar(body[0])) {
		won = true;
	}
	body[0].draw();
}

void Player::keyPressed(char ch) {
	size_t index = 0;
	for (char key : keys) {
		if (std::tolower(key) == std::tolower(ch)) {
			body[0].changeDir(Direction::directions[index]);
			break;
		}
		++index;
	}
}
