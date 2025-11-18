#include "GameManger.h"
#include <windows.h>
#include <conio.h>
#include <iostream>
#include "Player.h"
#include "Screen.h"
#include "utils.h"
#include <filesystem> 
#include <string> 

void handleRiddle(Screen& screen, Player& player)//TODO: REMOVE THIS FROM MAIN AND ADD IT TO A CLASS
{

	while (true) {
		char ch = _getch();          // blocks until a key

		// ignore everything that is not 1–4
		if (ch < '1' || ch > '4')
			continue;

		bool correct = false;

		// *** choose the correct answer here ***
		if (ch == '1') {
			correct = true;
		}

		if (correct) {
			// 1. reload the main level
			screen.restoreBackup();
			screen.draw();

			// 2. exit riddle mode for this player
			player.Change_Riddle(false);   // add a simple setter in Player

			break;   // leave handleRiddle, game loop resumes
		}
		else {
			// simple feedback + retry
			gotoxy(8, 22);
			std::cout << "Wrong answer, try again (1-4)..." << std::flush;
			// loop continues and waits for next key
		}
	}
}



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
				if (player.inRiddle()) {
					handleRiddle(theScreen, player);

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

