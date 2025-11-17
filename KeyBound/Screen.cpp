#include <iostream>

#include "Screen.h"
#include "utils.h"

#include <fstream> 


bool Screen::loadFromFile(const std::string& filename)
{
    std::ifstream file(filename);   // פתיחת הקובץ לקריאה (ios::in הוא ברירת מחדל)

    if (!file) {
        std::cerr << "Error: cannot open map file: " << filename << std::endl;
        return false;
    }

    std::string line;

    for (int y = 0; y <= MAX_Y; ++y) {
        if (!std::getline(file, line)) {
            std::cerr << "Error: not enough lines in map file: " << filename << std::endl;
            return false;
        }

        // אם השורה קצרה מדי – נשלים ברווחים
        if ((int)line.size() < MAX_X + 1) {
            line.resize(MAX_X + 1, ' ');
        }

        // נעתיק את התווים לשדה screen
        for (int x = 0; x <= MAX_X; ++x) {
            screen[y][x] = line[x];
        }
        screen[y][MAX_X + 1] = '\0';
    }


    return true;
}
void Screen::draw() const {
	int y = 0;
	for (const auto& row : screen) {
		gotoxy(0, y++);
		std::cout << row << std::flush;
	}
}
