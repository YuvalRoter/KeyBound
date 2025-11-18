#include "GameManger.h"
#include <windows.h>
#include <conio.h>
#include <iostream>
#include "Player.h"
#include "Screen.h"
#include "utils.h"
#include <filesystem> 
#include <string> 

int main() {
	
	constexpr char ESC = 27, EXIT = '9';
	hideCursor();
	cls();
	Screen theScreen;

	if (!theScreen.loadFromFile("menu.txt")) { // 1. Load and draw the KEYBOUND MENU from menu.txt
		std::cout << "Failed to load map!\n";
		return 1;
	}

	theScreen.draw();

	char choice = 0;
	while (true) {
		
		choice = _getch();              // blocks until a key is pressed

		if (choice == EXIT) {            // EXIT on the menu -> exit program
			cls();
			return 0;
		}

		if (choice >= '1' && choice <= '8') {
			break;                      // valid menu option
		}

		// any other key is ignored; loop continues
	}

	cls();

	//  load level 1 map
	if (!theScreen.loadFromFile("level1.txt")) {
		std::cout << "Failed to load level 1 map!\n";
		return 1;
	}

	theScreen.draw();


		Player players[] = {
			Player(Point(10, 10, {1, 0}, '@'), "wdsat", theScreen),
			Player(Point(5, 15, {-1, 0}, '&'), "86549", theScreen)
		};

		bool won = false;
		while (!won) {
			for (auto& player : players) {
				player.move();
				if (player.hasWon()) {
					Sleep(20);
					cls();
					std::cout << player.getChar() << " won!" << std::endl;
					(void)_getch();
					won = true;
					break;
				}
			}
			Sleep(50);
			if (_kbhit()) {
				char key = _getch();
				if (key == ESC) {
					// pause
					key = _getch();
					if (key == 'h' || key == 'H') {
						break;
					}
				}
				else {
					for (auto& player : players) {
						player.keyPressed(key);
					}
				}
			}
		}
		cls();



	
}