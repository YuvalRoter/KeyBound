#pragma once

#include "Point.h"
#include "Screen.h"

class Player {

	static constexpr size_t NUM_KEYS = 5;
	Point body;
	char keys[NUM_KEYS];
	Screen& screen;
	bool won = false;
public:
	Player(const Point& start_point, const char(&the_keys)[NUM_KEYS + 1], Screen& theScreen)
		: screen(theScreen)
	{
		
		body = start_point;
		
		memcpy(keys, the_keys, sizeof(keys[0]) * NUM_KEYS);
	}
	char getChar() const {
		return body.getChar();
	}
	void move();
	void keyPressed(char ch);
	bool hasWon() const {
		return won;
	}
	void jump(int NumberOfJumps = 3);
};

