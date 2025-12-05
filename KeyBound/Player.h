#pragma once

#include "Point.h"
#include "Screen.h"
#include "Direction.h"

class Player {

	static constexpr size_t NUM_KEYS = 5;
	Point body;
	char keys[NUM_KEYS];
	Direction dir;
	Screen& screen;
	int PlayerSpeed;
	bool won = false, Riddle = false, finishedLevel = false;;



public:


	struct Controls {
		char up;
		char right;
		char stay;
		char left;
		char pause;
	};


	Player(const Point& start_point,
		const Direction& start_dir,
		const Player::Controls& controls,
		Screen& theScreen,
		int speed = 1) // Default speed is 1 
		: body(start_point),       // 1. Initialize Body
		dir(start_dir),          // 2. Initialize Direction
		screen(theScreen),       // 3. Initialize Reference
		PlayerSpeed(speed)       // 4. Initialize Speed
	{
		body = start_point;
		keys[0] = controls.up;
		keys[1] = controls.right;
		keys[2] = controls.stay;
		keys[3] = controls.left;
		keys[4] = controls.pause;
	}
	char getChar() const {
		return body.getChar();
	}
	char getstaybutton() const {
		return keys[4];
	}
	void move();
	void keyPressed(char ch);
	bool hasWon() const {
		return won;
	}
	bool inRiddle() const {
		return Riddle;
	}
	void Change_Riddle(bool type) {
		Riddle = type;
	}

	Point getPoint() const {
		return body;
	}
	bool isFinished() const { return finishedLevel; }

	void setFinished(bool state) { finishedLevel = state; }

	void setPosition(const Point& p) {
		body = p;
	}

	void jump(int NumberOfJumps = 3);

	int getX() const {
		return body.getX();
	}
	int getY() const {
		return body.getY();
	}
	void setDirection(Direction d) {
		dir = d;
	}
};

