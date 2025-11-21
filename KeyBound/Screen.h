#pragma once

#include "Point.h"
#include <string>

// ---------------- def objects ---------------- // 

#define WALL '#'
#define RIDDEL '?'
#define DOOR 'D'
#define SPRING '+'
#define BOMB 'B'
#define PLAYER1 '@'
#define PLAYER2 '&'
#define WONCHAR '%'
#define HEARTCHAR '$'

class Screen {
public:
	static constexpr int MAX_X = 79;
	static constexpr int MAX_Y = 24;	

	static constexpr int SIMON_WIDTH = 16;  // columns
	static constexpr int SIMON_HEIGHT = 8;   // rows
	static constexpr int SIMON_GAP_X = 4;   // horizontal gap
	static constexpr int SIMON_GAP_Y = 2;   // vertical gap

	enum {
		LightBlock = 176,
		MediumBlock = 177,
		FullBlock = 219,
		DarkBlock = 178
		
	};

	enum {
		Black = 0,
		Blue = 1,
		Green = 2,
		Cyan = 3,
		Red = 4,
		Magenta = 5,
		Brown = 6,
		LightGray = 7,
		DarkGray = 8,
		LightBlue = 9,
		LightGreen = 10,
		LightCyan = 11,
		LightRed = 12,
		LightMagenta = 13,
		Yellow = 14,
		White = 15,
		BLANK = -1
	};
private:

	char screen[MAX_Y + 1][MAX_X + 2];

	char backup[MAX_Y + 1][MAX_X + 2];

	bool hasBackup = false;

	char charAt(const Point& p) const {
		return screen[p.getY()][p.getX()];
	}
	

public:

	void saveBackup();
	void restoreBackup();

	bool loadFromFileToMap(const std::string& filename);
	bool loadfile(const std::string& filename, std::ifstream& file);


	void setCell(int y, int x, char c) {
		screen[y][x] = c;
	}
	bool isWall(const Point& p) const {
		return charAt(p) == WALL;
	}
	bool isRiddle(const Point& p) const {
		return charAt(p) == RIDDEL;
	}
	bool isWonChar(const Point& p) const {
		return charAt(p) == WONCHAR;
	}
	bool isSpring(const Point& p) const {
		return charAt(p) == SPRING;
	}

	bool isDoor(const Point& p) const {
		return charAt(p) == DOOR;
	}

	void draw() const;

	void drawSimonSquare(int left, int top,char filler) const;

	void drawSimon(int litIndex)const;

	char getCharAt(int y, int x) const {
		return screen[y][x];
	}
};