#include <iostream>
#include "Point.h"
#include "utils.h"
#include "Screen.h"
#include "Direction.h" 


void Point::draw(char c) const {
    gotoxy(x, y);

    // Color by character
    if (c == Screen::PLAYER1) {
        setTextColor(Screen::Color::Cyan);
    }
    else if (c == Screen::PLAYER2) {
        setTextColor(Screen::Color::Red);
    }
    else if (c == Screen::TORCH) {
        // Optional: make the torch stand out
        setTextColor(Screen::Color::Yellow);
    }
    else if (c == Screen::OBSTACLE) {
		setTextColor(Screen::Color::DarkGray);
    }
    else {
        setTextColor(Screen::Color::LightGray);
    }

    std::cout << c;

    // Reset so HUD / other text remains consistent
    setTextColor(Screen::Color::LightGray);
}

void Point::draw() const {
    draw(ch);
}
Point Point::operator+(const Direction& d) const {
    
    return Point(x + d.getDirX(), y + d.getDirY(), ch);
}
