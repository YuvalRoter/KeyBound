#include "GameManger.h"
#include <windows.h>
#include <conio.h>
#include <iostream>
#include <string>
#include <random>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include "utils.h"
#include "Riddle.h"
#include "Room.h"
#include "Screen.h"

// ===========================
//      Internal Constants
// ===========================
namespace {
	// Random Number Generator setup
	std::mt19937 rng(std::random_device{}());

	// Game Configuration
	constexpr int START_X = 6;
	constexpr int START_Y = 5; // derived from START-1
	constexpr int MAX_SIMON_LENGTH = 4;
	constexpr int MIN_SIMON_LENGTH = 3;
	constexpr int SIMON_DELAY_MS = 400;
	constexpr int FOG_RADIUS_WITH_TORCH = 4;
	constexpr int FOG_RADIUS_NO_TORCH = 0;
	constexpr int WIN_POINTS = 50;

	// Menu Options 
	constexpr int MENU_OPT_START = 1;      // Starts game
	constexpr int MENU_OPT_SETTINGS = 3;   // Opens sub-menu
	constexpr int MENU_OPT_GUIDE = 4;      // Opens guide

	// Player Configuration
	constexpr Player::Controls P1_KEYS = { 'w', 'd', 's', 'a', ' ' };
	constexpr Player::Controls P2_KEYS = { 'i', 'l', 'k', 'j', 'm' };
	constexpr char P1_DROP_KEY = 'e';
	constexpr char P2_DROP_KEY = 'O';
}

// ===========================
//    Static Door Locations
// ===========================
// Define the physical coordinates for the doors
const Point GameManger::initialDoorLocations[MAX_DOORS] = {
	Point(79, 2),   // [0] Right Side (Used for Level 1 -> Level 2)
	Point(36, 1),   // [1] Top Side   (Used for Level 1 -> Level 3)
	Point(38, 24),  // [2] Bottom Side (Used for Level 1 -> Final)
	Point(36, 24),    // [3] Left Side  (Used for Level 2 Secret Door)
	Point(0, 2),    // [4] Left Side (Used for Level 1 <- Level 2)
	Point(36, 24)   // [5] Bottom Side (Used for Level 1 <- Level 3)
};

// Helper for random numbers
static int randomInt(int min, int max) {
	std::uniform_int_distribution<int> dist(min, max);
	return dist(rng);
}

// ===========================
//      Constructor
// ===========================
GameManger::GameManger() :
	currentRoom(-1),
	players{
		Player(Point(START_X, START_Y, Screen::PLAYER1), Direction::directions[Direction::STAY], P1_KEYS, screen),
		Player(Point(START_X - 2, START_Y, Screen::PLAYER2), Direction::directions[Direction::STAY], P2_KEYS, screen)
	}
{
	hideCursor();
	cls();
	initRooms();
	initDoors();

	// Attempt to load questions, handle error if missing
	if (!loadQuestionsFromFile("questions.txt")) {
		// Optional: Log error here if you had a logger
	}
}

// ===========================
//      Visibility Logic
// ===========================

// Calculates Chebyshev distance to determine if a tile is visible
static bool visibleFromPlayer(const Player& p, int x, int y) {
	int radius = p.hasTorch() ? FOG_RADIUS_WITH_TORCH : FOG_RADIUS_NO_TORCH;

	Point pp = p.getPoint();
	int dx = std::abs(x - pp.getX());
	int dy = std::abs(y - pp.getY());

	// Chebyshev distance (chessboard distance)
	int distance = (dx > dy) ? dx : dy;

	return distance <= radius;
}

void GameManger::drawWithFog() {
	// 0. On first use in this room, mark everything as "unknown"
	if (!fogInitialized) {
		for (int y = 0; y <= Screen::MAX_Y; ++y) {
			for (int x = 0; x <= Screen::MAX_X; ++x) {
				fogLastFrame[y][x] = '\0';   // impossible screen char
			}
		}
		fogInitialized = true;
	}

	// 1. Cache player positions
	Point p1 = players[0].getPoint();
	Point p2 = players[1].getPoint();

	// IMPORTANT: do NOT touch y == 0 (HUD row).
	// Row 0 is owned by printStatsBar().
	for (int y = 1; y <= Screen::MAX_Y; ++y) {
		for (int x = 0; x <= Screen::MAX_X; ++x) {

			bool visible =
				visibleFromPlayer(players[0], x, y) ||
				visibleFromPlayer(players[1], x, y);

			// 2. "raw" character: what's logically on this tile now?
			char raw = ' ';

			if (visible) {
				if (p1.getX() == x && p1.getY() == y) {
					raw = players[0].getChar();
				}
				else if (p2.getX() == x && p2.getY() == y) {
					raw = players[1].getChar();
				}
				else {
					raw = screen.getCharAt(y, x); // map: wall, floor, key, etc.
				}
			}

			// 3. Convert raw + visibility into display char + color
			char displayChar;
			Screen::Color color;

			if (!visible) {
				displayChar = ' ';
				color = Screen::Color::LightGray;
			}
			else if (raw == Screen::PLAYER1) {
				displayChar = raw;
				color = Screen::Color::Cyan;
			}
			else if (raw == Screen::PLAYER2) {
				displayChar = raw;
				color = Screen::Color::Red;
			}
			else if (raw == Screen::WALL) {
				displayChar = static_cast<char>(Screen::BlockType::MediumBlock);
				color = Screen::Color::Brown;
			}
			else if (raw == Screen::WONCHAR) {
				displayChar = static_cast<char>(Screen::BlockType::DarkBlock);
				color = Screen::Color::LightRed;
			}
			else {
				displayChar = raw;
				color = Screen::Color::LightGray;
			}

			// 4. If nothing changed since last fog frame, skip this cell
			if (fogLastFrame[y][x] == displayChar) {
				continue;
			}

			// 5. Otherwise update console + buffer
			fogLastFrame[y][x] = displayChar;

			gotoxy(x, y);
			setTextColor(static_cast<unsigned short>(color));
			std::cout << displayChar;
		}
	}

	// 6. Reset color so HUD and other text are correct
	setTextColor(static_cast<unsigned short>(Screen::Color::LightGray));
}


// ===========================
//      Game Flow
// ===========================



void GameManger::run() {
	if (!showMenu()) {
		return; // User chose EXIT
	}

	// Start at the first level (Room 0)
	loadRoom(0);

	gameLoop();
}


// ===========================
//      Menu Helper Functions
// ===========================

void GameManger::drawSettingsMenu() {
	screen.loadFromFileToMap("menu.txt");
	screen.draw();


	int cx = Screen::MAX_X / 2;
	int cy = Screen::MAX_Y / 3;

	if (g_colorsEnabled) setTextColor(Screen::Color::Cyan);
	gotoxy(cx - 10, cy);     std::cout << "=== COLOR SETTINGS ===";
	if (g_colorsEnabled) setTextColor(Screen::Color::LightGray);

	gotoxy(cx - 10, cy + 2); std::cout << "1. Enable Colors";
	gotoxy(cx - 10, cy + 3); std::cout << "2. Disable Colors (B&W)";
	gotoxy(cx - 10, cy + 5); std::cout << "3. Back to Main Menu";

	gotoxy(cx - 10, cy + 8);
	std::cout << "Current Mode: ";
	if (g_colorsEnabled) {
		setTextColor(Screen::Color::Green);
		std::cout << "COLOR ON";
	}
	else {
		setTextColor(Screen::Color::DarkGray);
		std::cout << "COLOR OFF";
	}
	setTextColor(Screen::Color::LightGray);
}

bool GameManger::showMenu() {
	while (true) {
		screen.loadFromFileToMap("menu.txt");
		screen.draw();
		printMainMenu();

		int choice = NumbersInput();

		switch (choice) {
		case 1:
			cls();
			return true;

		case 2:
			printInstructions();

			break;

		case 3:
			while (true) {
				drawSettingsMenu();
				int subChoice = NumbersInput();

				if (subChoice == 1) {
					g_colorsEnabled = true;
				}
				else if (subChoice == 2) {
					g_colorsEnabled = false;
				}
				else if (subChoice == 3) {
					break;
				}
			}
			break;

		case 4:
			printControls();
			break;

		case 9:
			cls();
			std::cout << "Goodbye!\n";
			return false;

		default:
			break;
		}
	}
}

void GameManger::printMainMenu() {
	// 1. Draw the global frame first

	int cx = Screen::MAX_X / 2;
	int cy = 6;
	// 2. Title
	if (g_colorsEnabled) setTextColor(Screen::Color::Cyan);
	gotoxy(cx - 12, cy);     std::cout << "########################";
	gotoxy(cx - 12, cy + 1); std::cout << "#      KeyBound        #";
	gotoxy(cx - 12, cy + 2); std::cout << "########################";

	if (g_colorsEnabled) setTextColor(Screen::Color::LightGray);

	// 3. Options
	int optX = cx - 10;

	gotoxy(optX, cy + 5); std::cout << "1. Start a New Game";
	gotoxy(optX, cy + 7); std::cout << "2. Instructions (Rules)";
	gotoxy(optX, cy + 9); std::cout << "3. Choose Color Mode";
	gotoxy(optX, cy + 11); std::cout << "4. Controls Guide";
	gotoxy(optX, cy + 14); std::cout << "9. Exit Game";

	// 4. Footer
	if (g_colorsEnabled) setTextColor(Screen::Color::DarkGray);
	gotoxy(optX - 5, Screen::MAX_Y - 3);
	std::cout << "Select an option using number keys...";
	if (g_colorsEnabled) setTextColor(Screen::Color::LightGray);
}

void GameManger::printInstructions() {

	screen.loadFromFileToMap("menu.txt");
	screen.draw();

	int cx = Screen::MAX_X / 2;
	int startY = 5;
	int leftAlign = 15; // Indent from left border

	if (g_colorsEnabled) setTextColor(Screen::Color::Yellow);
	gotoxy(cx - 10, startY); std::cout << "=== INSTRUCTIONS ===";
	if (g_colorsEnabled) setTextColor(Screen::Color::LightGray);

	gotoxy(leftAlign, startY + 3); std::cout << "1. You control two players simultaneously.";
	gotoxy(leftAlign, startY + 5); std::cout << "2. Navigate through the maze rooms.";
	gotoxy(leftAlign, startY + 7); std::cout << "3. Collect KEYS to open DOORS.";
	gotoxy(leftAlign, startY + 9); std::cout << "4. Solve Riddles (Simon Says & Math) to progress.";



	gotoxy(leftAlign, Screen::MAX_Y - 3);
	std::cout << "Press 1 to go to the next page or any key to go back...";
	int choice = NumbersInput();
	if (choice == 1) {
		screen.loadFromFileToMap("SimonGuide.txt");
		screen.draw();
		int choice = NumbersInput();
	}

}

void GameManger::printControls() {

	screen.loadFromFileToMap("menu.txt");
	screen.draw();
	// 1. Define Layout Coordinates
	int cx = Screen::MAX_X / 2;
	int col1 = 15; // X position for Player 1
	int col2 = 45; // X position for Player 2
	int row = 4;   // Starting Y position

	// 2. Draw Title (Centered)
	if (g_colorsEnabled) setTextColor(Screen::Color::Green);
	gotoxy(cx - 9, row); std::cout << "=== CONTROLS GUIDE ===";
	if (g_colorsEnabled) setTextColor(Screen::Color::LightGray);

	row += 3; // Move down

	// 3. Draw Column Headers
	if (g_colorsEnabled) setTextColor(Screen::Color::Cyan);
	gotoxy(col1, row); std::cout << "PLAYER 1 (Left):";
	gotoxy(col2, row); std::cout << "PLAYER 2 (Right):";
	if (g_colorsEnabled) setTextColor(Screen::Color::LightGray);

	row += 2; // Move down for keys

	// 4. Draw Keys (Row by Row for perfect alignment)
	gotoxy(col1, row);   std::cout << "[W] Up";
	gotoxy(col2, row++); std::cout << "[I] Up";

	gotoxy(col1, row);   std::cout << "[S] Down";
	gotoxy(col2, row++); std::cout << "[K] Down";

	gotoxy(col1, row);   std::cout << "[A] Left";
	gotoxy(col2, row++); std::cout << "[J] Left";

	gotoxy(col1, row);   std::cout << "[D] Right";
	gotoxy(col2, row++); std::cout << "[L] Right";

	gotoxy(col1, row);   std::cout << "[SPC] Stop Move";
	gotoxy(col2, row++); std::cout << "[M]   Stop Move";

	gotoxy(col1, row);   std::cout << "[E] Drop Torch";
	gotoxy(col2, row++); std::cout << "[O] Drop Torch";

	// 5. General Section (Lower down)
	row += 3;
	if (g_colorsEnabled) setTextColor(Screen::Color::Yellow);
	gotoxy(col1, row++); std::cout << "GENERAL:";
	if (g_colorsEnabled) setTextColor(Screen::Color::LightGray);

	gotoxy(col1, row);   std::cout << "[ESC] Pause / Exit Menu";

	// 6. Footer
	if (g_colorsEnabled) setTextColor(Screen::Color::DarkGray);
	gotoxy(col1, Screen::MAX_Y - 2);
	std::cout << "Press any key to go back...";
	if (g_colorsEnabled) setTextColor(Screen::Color::LightGray);

	(void)_getch();
}

void GameManger::gameLoop()
{


	while (running && !won) {

		handleInput();
		updatePlayers();

		if (rooms[currentRoom].dark) {
			// Incremental fog redraw
			drawWithFog();
		}

		// HUD (row 0) – safe because drawWithFog does not touch it
		printStatsBar();

		// One consistent delay per frame
		Sleep(60);   // tweak 50–70 to taste
	}

	cls();
}


void GameManger::handleInput() {
	if (!_kbhit()) return;

	char key = _getch();

	// Handle Global Keys (ESC)
	if (key == 27) { // ASCII for ESC
		// Check for double ESC or specific exit sequence if needed, 
		// for now, strict ESC exit:
		if (_kbhit()) {
			char next = _getch(); // consume extra chars if any
		}

		// Simple Pause/Exit menu could go here. For now, we set running false.
		// running = false; // Uncomment to enable ESC to quit

		// Your logic: ESC + H to exit
		key = _getch();
		if (key == 'h' || key == 'H') {
			running = false;
		}
	}
	else {
		// Handle Action Keys
		if (key == P1_DROP_KEY || key == std::toupper(P1_DROP_KEY)) {
			players[0].dropTorch();
			printStatsBar();
		}
		else if (key == P2_DROP_KEY || key == std::toupper(P2_DROP_KEY)) {
			players[1].dropTorch();
			printStatsBar();
		}
		else {
			// Pass movement keys to players
			for (auto& player : players) {
				player.keyPressed(key);
			}
		}
	}
}

// ===========================
//      Update Logic
// ===========================

void GameManger::updatePlayers() {
	bool allFinished = true;

	for (auto& player : players) {
		// 1. Move logic
		if (!player.isFinished()) {
			player.move(globalDoors, MAX_DOORS, currentRoom);

			// Check if HUD update is requested (e.g., key collected)
			if (player.getHUD()) {
				player.setHud(false);
				printStatsBar();
			}

			if (!player.isFinished()) {
				allFinished = false;
			}
		}

		// 2. Riddle Logic
		if (player.inRiddle()) {
			// Stop movement
			player.keyPressed(player.getstaybutton());
			handleRiddle(player);
			printStatsBar();
		}
		if (player.hasWon()) {
			won = true; 
			cls();      // Clear the maze
				
			// Play the animation 3 times
			for (int cycle = 0; cycle < 5; cycle++) {

				// Array of filenames to keep code clean
				std::string frames[] = {
					"fireworks1.txt",
					"fireworks2.txt",
					"fireworks3.txt",
					"fireworks4.txt"
				};

				for (const auto& fileName : frames) {
					screen.loadFromFileToMap(fileName);
					screen.draw();

					
					Sleep(200);
				}
			}

			
			Sleep(2000);
		}



		
	}

	// 3. Room Transition (Synchronized)
	if (allFinished) {
		int destination = -1;

		// Determine target room based on which door was taken.
		// P2 takes precedence if both trigger (Game Design choice).
		if (players[0].getTargetRoom() != -1) {
			destination = players[0].getTargetRoom();
		}
		if (players[1].getTargetRoom() != -1) {
			destination = players[1].getTargetRoom();
		}

		if (destination >= 0 && destination < NUMBER_OF_ROOMS) {
			loadRoom(destination);
			for (auto& player : players) {
				player.resetLevelData();
			}
		}
		else {
			// Fallback: Restart Level 1
			loadRoom(0);
			for (auto& player : players) player.resetLevelData();
		}
	}
}

// ===========================
//      Initialization
// ===========================

void GameManger::initRooms()
{
	// Initialize Rooms with file names, spawn points and lighting settings
	rooms[0] = Room("level1.txt", { Point(10, 4, Screen::PLAYER1), Point(7, 4, Screen::PLAYER2) }, false);
	rooms[1] = Room("level2.txt", { Point(3, 3, Screen::PLAYER1),  Point(3, 5, Screen::PLAYER2) }, true);
	rooms[2] = Room("level3.txt", { Point(35, 23, Screen::PLAYER1),  Point(36, 23, Screen::PLAYER2) }, false);
	rooms[3] = Room("levelFinal.txt", { Point(3, 3, Screen::PLAYER1), Point(3, 5, Screen::PLAYER2) }, false);
}

void GameManger::initDoors() {
	// Format: { Position, ID, SourceRoom, TargetRoom, KeysCost, IsOpen }

	// ==========================================
	//          LEVEL 1 DOORS (The Hub)
	// ==========================================

	// Door 0: Goes to Level 2 (Location: Right Side)
	globalDoors[0] = { initialDoorLocations[0], 0, 0, 1, 1, false };

	// Door 1: Goes to Level 3 (Location: Top Side)
	globalDoors[1] = { initialDoorLocations[1], 1, 0, 2, 1, false };

	// Door 2: Goes to Final Level (Location: Bottom Side)
	globalDoors[2] = { initialDoorLocations[2], 2, 0, 3, 5, false };


	// ==========================================
	//              LEVEL 2 DOORS
	// ==========================================

	// Door 3: Back to Level 1 (Location: Right Side - SAME AS ENTERING)
	globalDoors[3] = { initialDoorLocations[4], 3, 1, 0, 0, true }; // 0 cost, already open

	// Door 4: Secret Door (Location: Left Side) -> SecretRoom
	globalDoors[4] = { initialDoorLocations[3], 4, 1, 3, 6, false };


	// ==========================================
	//              LEVEL 3 DOORS
	// ==========================================

	// Door 5: Back to Level 1 (Location: Top Side - SAME AS ENTERING)
	globalDoors[5] = { initialDoorLocations[5], 5, 2, 0, 0, true }; // 0 cost, already open
}

void GameManger::loadRoom(int index)
{
	// 1. Save state of current room
	if (currentRoom >= 0 && currentRoom < NUMBER_OF_ROOMS) {
		screen.saveScreenToRoom(rooms[currentRoom]);
	}

	currentRoom = index;

	// 2. Load map for new room (from memory or file)
	if (rooms[currentRoom].isVisited) {
		screen.loadScreenFromRoom(rooms[currentRoom]);
	}
	else {
		screen.loadFromFileToMap(rooms[currentRoom].mapFile);
	}

	// 3. Render Doors
	for (int i = 0; i < MAX_DOORS; ++i) {
		if (globalDoors[i].sourceRoomIndex == currentRoom) {
			int x = globalDoors[i].position.getX();
			int y = globalDoors[i].position.getY();
			// Show keys required (e.g., '1', '2') on the door tile
			screen.setCell(y, x, '0' + globalDoors[i].KeysToOpen);
		}
	}

	// 4. Reset Player Positions
	const auto& starts = rooms[index].startPositions;
	for (std::size_t i = 0; i < NUMBER_OF_PLAYERS; ++i) {
		players[i].setFinished(false);
		if (i < starts.size()) {
			players[i].setPosition(starts[i]);
		}
	}

	// 5. Reset fog buffer for the new room
	fogInitialized = false;

	// 6. First draw for the new room
	if (rooms[currentRoom].dark) {
		drawWithFog();          // fog rendering for dark rooms
	}
	else {
		screen.draw();          // normal full draw for bright rooms
	}

	printStatsBar();
}

// ===========================
//      Riddles & Events
// ===========================

Riddle GameManger::generateRandomRiddle() {
	int r = randomInt(0, 1);
	size_t simonSize = randomInt(MIN_SIMON_LENGTH, MAX_SIMON_LENGTH);

	if (r == 0) { // Multiple Choice
		if (numQuestions == 0) {
			return Riddle::makeMultipleChoice("ERROR: NO QUESTIONS", { "0", "0", "0", "0" }, 0);
		}
		int idx = randomInt(0, numQuestions - 1);
		const Question& q = questions[idx];
		return Riddle::makeMultipleChoice(q.text, q.options, q.correctIndex);
	}
	else { // Simon Says
		std::vector<int> pattern(simonSize);
		for (size_t i = 0; i < simonSize; ++i) {
			pattern[i] = randomInt(0, 3);
		}
		return Riddle::makeSimonSays(pattern, SIMON_DELAY_MS);
	}
}

void GameManger::handleRiddle(Player& player) {
	Riddle r = generateRandomRiddle();

	// Load generic riddle background
	screen.loadFromFileToMap("riddle1.txt");

	if (r.getType() == RiddleType::MultipleChoice) {
		handleMulti(r, player);
	}
	else {
		handleSimon(r, player);
	}

	// Restore map state
	screen.restoreBackup();

	if (rooms[currentRoom].dark) {
		// Force a fresh fog draw after the riddle screen
		fogInitialized = false;
		drawWithFog();
	}
	else {
		screen.draw();
	}

	printStatsBar();

	// Release player from riddle state
	player.setInRiddle(false);
}

void GameManger::handleSimon(Riddle& riddle, Player& player)
{
	std::vector<int> pattern = riddle.getSimonPattern();
	int flashDelayMs = riddle.getSimonDelayMs();

	// 1. Show Pattern
	for (int idx : pattern) {
		screen.drawSimon(-1); // Clear
		Sleep(150);
		screen.drawSimon(idx); // Flash
		Sleep(flashDelayMs);
		screen.drawSimon(-1); // Clear
		Sleep(150);
	}

	// 2. User Input
	for (std::size_t i = 0; i < pattern.size(); ++i) {
		int digit = NumbersInput();
		int choiceIndex = digit - 1; // Map 1-4 to 0-3

		screen.drawSimon(choiceIndex);

		if (choiceIndex != pattern[i]) {
			// Failure
			cls();
			gotoxy(30, 12);
			std::cout << "YOU FAILED!";
			Sleep(1000);
			return;
		}
	}

	// Success
	increaseScore(WIN_POINTS);
}

void GameManger::handleMulti(Riddle& riddle, Player& player)
{
	cls();
	const int consoleWidth = Screen::MAX_X + 1;
	const int qX = (consoleWidth - (int)riddle.getQuestion().size()) / 2;
	const int qY = Screen::MAX_Y / 3;

	// Display Question
	gotoxy((std::max)(0, qX), qY);
	std::cout << riddle.getQuestion();

	// Display Options
	const auto& options = riddle.getOptions();
	for (std::size_t i = 0; i < options.size(); ++i) {
		std::string line = std::to_string(i + 1) + ". " + options[i];
		int x = (consoleWidth - (int)line.size()) / 2;
		gotoxy((std::max)(0, x), qY + 2 + (int)i * 2);
		std::cout << line;
	}

	// Input Validation
	int chosenIndex = -1;
	while (true) {
		int digit = NumbersInput();
		if (digit >= 1 && digit <= (int)options.size()) {
			chosenIndex = digit - 1;
			break;
		}
	}

	// Check Result
	if (chosenIndex != riddle.getCorrectIndex()) {
		cls();
		gotoxy(consoleWidth / 2 - 5, Screen::MAX_Y / 2);
		std::cout << "YOU FAILED!";
		Sleep(1000);
	}
	else {
		cls();
		gotoxy(consoleWidth / 2 - 5, Screen::MAX_Y / 2);
		std::cout << "CORRECT!";
		Sleep(700);
		// Note: You might want to add score here too
	}
}

void GameManger::increaseScore(int Points)
{
	cls();
	score += Points;
	int cx = Screen::MAX_X / 2;
	int cy = Screen::MAX_Y / 2;

	// Flash Colors
	Screen::Color colors[] = { Screen::Color::Yellow, Screen::Color::LightGreen, Screen::Color::LightCyan };

	for (int i = 0; i < 6; ++i) {
		setTextColor(colors[i % 3]);

		// Draw Box
		gotoxy(cx - 10, cy - 2); std::cout << "#####################";
		gotoxy(cx - 10, cy - 1); std::cout << "#  SIMON SAYS WIN!  #";
		gotoxy(cx - 10, cy);     std::cout << "#                   #";
		gotoxy(cx - 10, cy + 1); std::cout << "#    SCORE + " << Points << "     #";
		gotoxy(cx - 10, cy + 2); std::cout << "#####################";

		if (i < 3) Beep(400 + (i * 100), 50);
		Sleep(150);
	}

	setTextColor(Screen::Color::LightGray);
	Sleep(600);
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

void GameManger::printStatsBar() {
	int totalKeys = Player::getCollectedKeys();

	// Build Inventory String
	std::string inventory = "";
	if (players[0].hasTorch() || players[1].hasTorch()) {
		inventory += "[TORCH] ";
	}
	if (inventory.empty()) inventory = "[EMPTY]";

	gotoxy(0, 0);
	setTextColor(Screen::Color::Cyan);

	std::cout << "LEVEL: " << (currentRoom + 1)
		<< " | SCORE: " << score
		<< " | KEYS: " << totalKeys
		<< " | INV: " << inventory;

	// Clear remainder of line
	int currentLen = 50;
	for (int i = 0; i < (Screen::MAX_X - currentLen); i++) std::cout << " ";

	setTextColor(Screen::Color::LightGray);
}

bool GameManger::loadQuestionsFromFile(const std::string& filename)
{
	std::ifstream file;
	if (!openFileForRead(filename, file, "questions"))
		return false;

	// Use a while loop to prevent reading beyond file end
	int i = 0;
	while (i < MAX_QUESTIONS && file.peek() != EOF) {
		Question& q = questions[i];

		if (!std::getline(file, q.text)) break;
		for (int j = 0; j < 4; j++) {
			if (!std::getline(file, q.options[j])) break;
		}

		std::string indexLine;
		if (!std::getline(file, indexLine)) break;
		try {
			q.correctIndex = std::stoi(indexLine);
		}
		catch (...) {
			q.correctIndex = 0; // Default error fallback
		}

		if (!std::getline(file, q.explanation)) break;

		std::string empty;
		std::getline(file, empty); // Consume separator

		i++;
	}

	numQuestions = i; // Update actual count
	return true;
}