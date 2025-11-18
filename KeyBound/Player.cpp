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
