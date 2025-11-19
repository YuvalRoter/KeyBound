#include "GameManger.h"
#include <windows.h>
#include <conio.h>
#include "utils.h"
#include "Riddle.h"
#include <string>
#include "Room.h"

GameManger::GameManger(): 
	players{
        Player(Point(10, 4, Direction::directions[Direction::RIGHT], PLAYER1), {'w', 'd', 's', 'a', ' '}, screen),
        Player(Point(7, 4, Direction::directions[Direction::RIGHT], PLAYER2), {'i', 'l', 'k', 'j', 'm'}, screen)
} 

{
    hideCursor();
    cls();
	initRooms();
}


Riddle GameManger::generateRandomRiddle() {
	int r = rand() % 2;   // 0 or 1

	if (r == 0) {
		// --- Multiple choice ---
		return Riddle::makeMultipleChoice(
			"What is 2 + 2?",
			{ "4", "3", "5", "2" },
			0  // correct index
		);
	}
	else {
		// --- Simon Says ---
		std::vector<int> pattern = { 0, 1, 3, 2 };
		return Riddle::makeSimonSays(pattern, 400);
	}
}


void GameManger::run() {
	if (!showMenu()) {
		return;          // user chose EXIT
	}

	loadRoom(0);

	gameLoop();
}


bool GameManger::showMenu() {
	// 1. Load and draw menu
	loadMap("menu.txt");


	// 2. Wait for valid choice
	int Choice = NumbersInput();
	if (Choice == 1)
		g_colorsEnabled = true;
	else if (Choice == 3)
		g_colorsEnabled = false;

	cls();
	return true;               // continue game
}

void GameManger::initRooms()
{
	rooms[0] = Room(
		"level1.txt",
		Point(10, 4, Direction::directions[Direction::RIGHT], PLAYER1),
		Point(7, 4, Direction::directions[Direction::RIGHT], PLAYER2)
	);

	rooms[1] = Room(
		"level2.txt",
		Point(5, 10, Direction::directions[Direction::RIGHT], PLAYER1),
		Point(5, 12, Direction::directions[Direction::RIGHT], PLAYER2)
	);

	rooms[2] = Room(
		"level3.txt",
		Point(3, 3, Direction::directions[Direction::RIGHT], PLAYER1),
		Point(3, 5, Direction::directions[Direction::RIGHT], PLAYER2)
	);
}
void GameManger::loadRoom(int index)
{
	currentRoom = index;

	// load map
	loadMap(rooms[currentRoom].mapFile);


	// reset players to starting positions
	for (std::size_t i = 0; i < NUMBER_OF_PLAYERS; ++i) {
		players[i].setPosition(rooms[index].startPositions[i]);
	}

	screen.draw();
}


int GameManger::NumbersInput()
{
	char choice = 0;
	while (true) {
		choice = _getch();
		if (std::isdigit(choice))
			return choice - '0';   
	}
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
		if (key == 'h' || key == 'H') {// exit if escape is active
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

		if (screen.isDoor(player.getPoint())) {
			int next = (currentRoom + 1) % NUMBER_OF_ROOMS;
			loadRoom(next);
			return; // break update, map changed
		}

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
			
		}
	}
}

void GameManger::handleRiddle(Player& player) {

	Riddle r = generateRandomRiddle();
	

	screen.loadFromFile("riddle1.txt");   

	// 3. Now solve according to type:
	if (r.getType() == RiddleType::MultipleChoice) {
		handleMulti(r, player);
	}
	else {
		handleSimon(r, player);
	}

	// After finishing:
	screen.restoreBackup();
	screen.draw();
	player.Change_Riddle(false);
}


void GameManger::handleSimon(Riddle& riddle, Player& player)
{
	// 1. Get pattern from riddle
	std::vector<int> pattern = riddle.getSimonPattern();

	
	int flashDelayMs = riddle.getSimonDelayMs();

	// 2. SHOW the pattern (computer plays)
	for (int idx : pattern) {
		// first draw all squares, none highlighted
		screen.drawSimon(-1);
		Sleep(150);

		// highlight the one at 'idx'
		screen.drawSimon(idx);
		Sleep(flashDelayMs);

		// turn off again between steps
		screen.drawSimon(-1);
		Sleep(150);
	}

	// 3. ASK PLAYER to repeat the pattern
	for (std::size_t i = 0; i < pattern.size(); ++i) {

		int digit = NumbersInput();   // wait for user input
		int choiceIndex = digit - 1;  // we need it to be 0 , 1 ,2 ,3 

		// show what the user pressed
		screen.drawSimon(choiceIndex);

		// check correctness
		if (choiceIndex != pattern[i]) {
			// WRONG – show message and exit the riddle
			cls();
			gotoxy(30, 12);          // adjust position as you like
			std::cout << "YOU FAILED!";
			Sleep(1000);
			return;                  // handleRiddle will then restore map
		}
	}

	// 4. If we get here – success
	cls();
	gotoxy(30, 12);
	std::cout << "GOOD JOB!";
	Sleep(1000);
}

void GameManger::handleMulti(Riddle& riddle, Player& player)
{
	// 1. Prepare screen (border / clear)
	cls();

	const int consoleWidth = Screen::MAX_X + 1;

	const int correctIndex = riddle.getCorrectIndex(); // 0..3
	const auto& options = riddle.getOptions();      // vector<string>
	const std::string& Q = riddle.getQuestion();

	// 2. Print question in the middle (horizontally)
	int qX = (int)((consoleWidth - (int)Q.size()) / 2);
	if (qX < 0) qX = 0;     // just in case question is too long
	int qY = Screen::MAX_Y / 3;   // about one-third from top

	gotoxy(qX, qY);
	std::cout << Q;

	// 3. Print options below the question, numbered 1-4
	int optStartY = qY + 2;       // start two lines below question

	for (std::size_t i = 0; i < options.size(); ++i) {
		std::string line =
			std::to_string(i + 1) + ". " + options[i];   // "1. option text"

		int x = (int)((consoleWidth - (int)line.size()) / 2);
		if (x < 0) x = 0;

		int y = optStartY + (int)i * 2;  // 1 empty line between options

		gotoxy(x, y);
		std::cout << line;
	}

	// 4. Wait for valid input 1-4
	int chosenIndex = -1;
	while (true) {
		int digit = NumbersInput();      // must return 0..9

		if (digit >= 1 && digit <= (int)options.size()) {
			chosenIndex = digit - 1;     // map 1..4 -> 0..3
			break;
		}
		// otherwise ignore and keep waiting
	}

	// 5. Check answer
	if (chosenIndex != correctIndex) {
		// WRONG
		cls();                           // clear whole screen
		gotoxy(consoleWidth / 2 - 5, Screen::MAX_Y / 2);
		std::cout << "YOU FAILED!";
		Sleep(1000);
		return;                          // handleRiddle() will restore map
	}
	else {
		// Optional: show success message
		cls();
		gotoxy(consoleWidth / 2 - 5, Screen::MAX_Y / 2);
		std::cout << "CORRECT!";
		Sleep(700);
		return;
	}
}
