#pragma once
#include <iostream> 

class Direction;

class Point {
    int x, y;
    char ch = ' ';
public:
    Point() : x(0), y(0), ch(' ') {} // Default constructor

    // Constructor used by GameManager
    Point(int x1, int y1, char c) : x(x1), y(y1), ch(c) {}

    // Helper constructor
    Point(int x1, int y1) : x(x1), y(y1), ch('*') {}

    int getX() const { return x; }
    int getY() const { return y; }
    char getChar() const { return ch; }

    // Logic: New Point = Old Point + Vector
    Point operator+(const Direction& d) const;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }

    void draw(); // Only draw, no move
    void draw(char c);

  
};