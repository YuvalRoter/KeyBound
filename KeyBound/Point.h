#pragma once

// Forward declaration: 
// Allows us to use 'Direction' in function signatures without including the full header.
class Direction;

class Point {
private:
    int x;      // X coordinate (Column)
    int y;      // Y coordinate (Row)
    char ch;    // The character representation (e.g., '*', '@')

public:
    // ===========================
    //       Constructors
    // ===========================

    // Default constructor: Initializes to (0,0) with a space char
    Point() : x(0), y(0), ch(' ') {}

    // Fully parameterized constructor
    Point(int x1, int y1, char c) : x(x1), y(y1), ch(c) {}

    // Helper constructor: Sets coordinates but defaults char to '*'
    Point(int x1, int y1) : x(x1), y(y1), ch('*') {}

    // ===========================
    //        Accessors
    // ===========================
    int getX() const { return x; }
    int getY() const { return y; }
    char getChar() const { return ch; }

    // ===========================
    //        Operators
    // ===========================

    // Returns a new Point resulting from moving this Point in Direction 'd'
    Point operator+(const Direction& d) const;

    // Checks if two points share the exact same coordinates
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }

    // ===========================
    //        Rendering
    // ===========================

    // Draws the point's stored character 'ch' at (x,y)
    void draw();

    // Draws a specific character 'c' at (x,y) (useful for erasing)
    void draw(char c);
};