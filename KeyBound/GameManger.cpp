#include "GameManger.h"
#include <windows.h>
#include <conio.h>
#include "utils.h"
#include "Riddle.h"
#include <string>
#include "Room.h"
#include <random>

#include <fstream>
#include <sstream>


static std::mt19937 rng(std::random_device{}());
static constexpr Player::Controls P1_KEYS = { 'w', 'd', 's', 'a', ' ' };
static constexpr Player::Controls P2_KEYS = { 'i', 'l', 'k', 'j', 'm' };
static constexpr int START = 6,MAXSIMON = 4,MINSIMON =3;


static int randomInt(int min, int max) {
	std::uniform_int_distribution<int> dist(min, max);
	return dist(rng);
}



GameManger::GameManger():
	currentRoom(-1),
	players{
	   Player(Point(START, START-1, PLAYER1), Direction::directions[Direction::STAY], P1_KEYS, screen),
       Player(Point(START-2,START-1, PLAYER2), Direction::directions[Direction::STAY], P2_KEYS, screen)
}
{
    hideCursor();
    cls();
	initRooms();
	initDoors();
	loadQuestionsFromFile("questions.txt");
}


Riddle GameManger::generateRandomRiddle() {
	int r = randomInt(0,1);   // 0 or 1
	size_t SimonSize = randomInt(MINSIMON, MAXSIMON);; 
	if (r == 0) {
		// If no questions loaded, fallback so the game doesn't crash
		if (numQuestions == 0) {
			return Riddle::makeMultipleChoice(
				"ERROR: NO QUESTIONS LOADED",
				{ "0", "0", "0", "0" },
				0
			);
		}

		int idx = randomInt(0, numQuestions - 1);
		const Question& q = questions[idx];

		return Riddle::makeMultipleChoice(
			q.text,
			{ q.options[0], q.options[1], q.options[2], q.options[3] },
			q.correctIndex
		);
	}

	else {// Simon Says
		std::vector<int> pattern(SimonSize, 0);
		for (size_t i = 0; i < SimonSize; ++i) {
			int num = randomInt(0, 3);
			pattern[i] = num;
		}
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
	else if (Choice == 8)
	{	
		screen.loadFromFileToMap("Guide.txt");
		screen.draw();
		Choice = NumbersInput();
		if (Choice == 1)
			showMenu();
		else {
			screen.loadFromFileToMap("SimonGuide.txt");
			screen.draw();
		}


	}

	cls();
	return true;               // continue game
}

void GameManger::initRooms()
{
	


	rooms[0] = Room(
		"level1.txt",
		{
			Point(10, 4, PLAYER1), //MAGIC NUMBERS!!!!!
			Point(7, 4, PLAYER2)
		}
	);


	rooms[1] = Room(
		"level2.txt",
		{
			Point(5, 10, PLAYER1),
			Point(5, 12, PLAYER2)
		}
	);


	rooms[2] = Room(
		"level3.txt",
		{
			Point(3, 3, PLAYER1),
			Point(3, 5, PLAYER2)
		}
	);
}

void GameManger::initDoors() {
	// Format: { Position, ID, SourceRoom, TargetRoom, KeysCost, IsOpen }
	
	// --- DOOR 0: Located in Level 1 (Room 0) ---
	// Position: Top Right (79, 4)
	// Leads to: Room 1 (Level 2)
	globalDoors[0] = { Point(79, 3), 0, 0, 1, 1, false };

	// --- DOOR 1: Located in Level 2 (Room 1) ---
	// Position: Bottom Right area (57, 22)
	// Leads to: Room 2 (Level 3)
	globalDoors[1] = { Point(57, 22), 1, 1, 2, 1, false };

	// --- DOOR 2: Located in Level 3 (Room 2) ---
	// Position: Bottom Middle area (17, 22)
	// Leads to: Room 0 (Loop back to Level 1 or Win)
	globalDoors[2] = { Point(17, 22), 2, 2, 0, 1, false };

	// Initialize the rest as empty/inactive
	for (int i = 3; i < MAX_DOORS; i++) {
		globalDoors[i] = { Point(0,0), i, -1, -1, 0, false };
	}
}

void GameManger::loadRoom(int index)
{
	// 1. Save the state of the CURRENT room before leaving
	// We check if currentRoom is valid 
	if (currentRoom >= 0 && currentRoom < NUMBER_OF_ROOMS) {
		screen.saveScreenToRoom(rooms[currentRoom]);
	}

	// 2. Switch the index
	currentRoom = index;

	// 3. Load the NEW room
	if (rooms[currentRoom].isVisited) {
		// If we have been here before, load the SAVED state (with missing keys)
		screen.loadScreenFromRoom(rooms[currentRoom]);
	}
	else {
		// If this is the first time, load from the TEXT FILE
		screen.loadFromFileToMap(rooms[currentRoom].mapFile);
	}

	// 4. Reset Players (Positions only, keep inventory)
	const auto& starts = rooms[index].startPositions;

	for (std::size_t i = 0; i < NUMBER_OF_PLAYERS; ++i) {
		players[i].setFinished(false);
		// The inventory should persist.

		if (i < starts.size()) {
			players[i].setPosition(starts[i]);
		}
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
	bool allFinished = true;

for (auto& player : players) {
        
	// 1. Move logic
	if (!player.isFinished()) {
		player.move(globalDoors, MAX_DOORS, currentRoom);

		// If they are STILL not finished after moving, the level isn't done.
		if (!player.isFinished()) {
			allFinished = false;
		}
	}
        
        if (player.inRiddle()) {
            player.keyPressed(player.getstaybutton());
            handleRiddle(player);
        }
    }

    // 2. SYNCHRONIZATION POINT
    // Only if ALL players have reached the door do we change the room
    if (allFinished) {
		int destination = -1;

		// LOGIC: Check who has a valid target room. 
		// We check Player 1, then Player 2. 
		// If Player 2 has a target, it overrides Player 1 (following the "Second Player" rule).

		if (players[0].getTargetRoom() != -1) {
			destination = players[0].getTargetRoom();
		}
		if (players[1].getTargetRoom() != -1) {
			destination = players[1].getTargetRoom();
		}

		// 3. EXECUTE LEVEL SWITCH
		if (destination >= 0 && destination < NUMBER_OF_ROOMS) {
			loadRoom(destination);

			for (auto& player : players) {
				player.resetLevelData();
			}
		}
		else {
			// Fallback (e.g., if something went wrong, restart level 1)
			loadRoom(0);
			for (auto& player : players) player.resetLevelData();
		}
    }
}
void GameManger::handleRiddle(Player& player) {


	Riddle r = generateRandomRiddle();
	
	screen.loadFromFileToMap("riddle1.txt");   

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

//this is a bouns part for a nice riddle simon game
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

bool GameManger::loadQuestionsFromFile(const std::string& filename)
{
	std::ifstream file;
	if (!openFileForRead(filename, file, "questions"))
		return false;

	for (int i = 0; i < 100; i++) {
		Question& q = questions[i];

		// Read question text
		if (!std::getline(file, q.text)) return false;

		// Read 4 options
		for (int j = 0; j < 4; j++) {
			if (!std::getline(file, q.options[j])) return false;
		}

		// Read correct index
		std::string indexLine;
		if (!std::getline(file, indexLine)) return false;
		q.correctIndex = std::stoi(indexLine);

		// Read explanation
		if (!std::getline(file, q.explanation)) return false;

		// Read the empty separator line
		std::string empty;
		std::getline(file, empty);
	}

	return true;
}
