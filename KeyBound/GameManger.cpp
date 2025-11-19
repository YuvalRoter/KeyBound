#include "GameManger.h"
#include <windows.h>
#include <conio.h>
#include "utils.h"


GameManger::GameManger()
    : players{
        Player(Point(10, 4, Direction::directions[Direction::RIGHT], '@'), {'w', 'd', 's', 'a', ' '}, screen),
        Player(Point(7, 4, Direction::directions[Direction::RIGHT], '*'), {'i', 'l', 'k', 'j', 'm'}, screen)
    }
{
    hideCursor();
    cls();
}


void GameManger::run() {
	if (!showMenu()) {
		return;          // user chose EXIT
	}

	loadMap("level1.txt");

	gameLoop();
}


bool GameManger::showMenu() {
	// 1. Load and draw menu
	loadMap("menu.txt");


	// 2. Wait for valid choice
	char choice = 0;
	while (true) {
		choice = _getch();

		if (choice == EXIT) { // '9'
			cls();
			return false;      // user chose to exit game
		}

		if (choice == '1') {           
			g_colorsEnabled = false;
			break;
		}

		if (choice == '3') {           
			g_colorsEnabled = true;
			break;
		}
	}

	cls();
	return true;               // continue game
}




void GameManger::gameLoop() {
	while (running && !won) {
		updatePlayers();  // move & check win / riddle
		handleInput();    // keyboard / pause
		Sleep(50);
	}

	cls(); // clear screen at the end of the level
}

	

void GameManger::handleInput() {
	if (!_kbhit()) return;

	char key = _getch();
	if (key == ESC) {
		key = _getch();
		if (key == 'h' || key == 'H') {
			running = false;
		}
	}
	else {
		for (auto& player : players) {
			player.keyPressed(key);
		}
	}
}




void GameManger::updatePlayers() {
	for (auto& player : players) {
		player.move();

		if (player.hasWon()) {
			Sleep(20);
			cls();
			std::cout << player.getChar() << " won!" << std::endl;
			(void)_getch();
			won = true;
			return; // leave updatePlayers, gameLoop will exit
		}

		if (player.inRiddle()) {
			handleRiddle(player);
			// after this returns, main level screen should be restored
		}
	}
}

void GameManger::handleRiddle(Player& player) {
	while (true) {
		char ch = _getch();

		// ignore everything that is not 1–4
		if (ch < '1' || ch > '4')
			continue;

		bool correct = false;

		// *** for now, hard-coded correct answer ***
		if (ch == '1') {
			correct = true;
		}

		if (correct) {
			// 1. reload the main level from backup
			screen.restoreBackup();
			screen.draw();

			// 2. exit riddle mode for this player
			player.Change_Riddle(false);

			break;
		}
		else {
			gotoxy(8, 22);
			std::cout << "Wrong answer, try again (1-4)..." << std::flush;
		}
	}
}
