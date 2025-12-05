#include <iostream>
#include "Point.h"
#include "utils.h"
#include "Screen.h"
#include "Direction.h" 

void Point::draw(char c) {
    gotoxy(x, y);
    std::cout << c;
}

void Point::draw() {
    draw(ch);
}

Point Point::operator+(const Direction& d) const {
    
    return Point(x + d.getDirX(), y + d.getDirY(), ch);
}
