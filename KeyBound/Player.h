#pragma once

#include "Point.h"
#include "Screen.h"

class Player {
	static constexpr size_t SIZE = 1;
	static constexpr size_t NUM_KEYS = 5;
	Point body[SIZE];
	char keys[NUM_KEYS];
	Screen& screen;
	bool won = false;

public:
	Player(const Point& start_point, const char(&the_keys)[NUM_KEYS + 1], Screen& theScreen)
		: screen(theScreen)
	{
		for (auto& p : body) {
			p = start_point;
		}
		memcpy(keys, the_keys, sizeof(keys[0]) * NUM_KEYS);
	}
	char getChar() const {
		return body[0].getChar();
	}
	void move();
	void keyPressed(char ch);
	bool hasWon() const {
		return won;
	}
};

