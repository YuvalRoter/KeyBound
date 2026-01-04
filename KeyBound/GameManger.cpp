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
#include <filesystem>
#include "utils.h"
#include "Riddle.h"
#include "Room.h"
#include "Screen.h"



// ===========================
//      Internal Constants
// ===========================
namespace fs = std::filesystem;
namespace {
	// Random Number Generator setup
	std::mt19937 rng(std::random_device{}());

	// Game Configuration
	constexpr int START_X = 6;
	constexpr int START_Y = 5; 
	constexpr int MAX_SIMON_LENGTH = 4;
	constexpr int MIN_SIMON_LENGTH = 3;
	constexpr int SIMON_DELAY_MS = 400;
	constexpr int FOG_RADIUS_WITH_TORCH = 4;
	constexpr int FOG_RADIUS_NO_TORCH = 0;
	constexpr int WIN_POINTS = 50;

	// Menu Options 
	constexpr int MENU_OPT_START = 1;      // Starts game
	constexpr int MENU_OPT_LOAD = 2;       // Load game
	constexpr int MENU_OPT_INSTRUCTIONS = 3;
	constexpr int MENU_OPT_SETTINGS = 4;
	constexpr int MENU_OPT_GUIDE = 5;

	// Player Configuration
	constexpr Player::Controls P1_KEYS = { 'w', 'd', 's', 'a', ' ' };
	constexpr Player::Controls P2_KEYS = { 'i', 'l', 'k', 'j', 'm' };
	constexpr char P1_DROP_KEY = 'e';
	constexpr char P2_DROP_KEY = 'o';
}

// helper struct
struct LevelMetadata {
	std::string filename;
	Point spawnP1;
	Point spawnP2;
	bool isDark;
};
std::vector<LevelMetadata> getLevelDefs() {
	return {
		{"level1.txt",     Point(6, 4, Screen::PLAYER1),   Point(6, 3, Screen::PLAYER2),   false},
		{"level2.txt",     Point(3, 3, Screen::PLAYER1),   Point(3, 5, Screen::PLAYER2),   true},
		{"level3.txt",     Point(35, 23, Screen::PLAYER1), Point(36, 23, Screen::PLAYER2), false},
		{"levelFinal.txt", Point(3, 3, Screen::PLAYER1),   Point(3, 5, Screen::PLAYER2),   false},
		{"level4.txt",     Point(23, 3, Screen::PLAYER1),  Point(22, 3, Screen::PLAYER2),  false}
	};
}

// ===========================
//    Static Door Locations
// ===========================
// Define the physical coordinates for the doors
const Point GameManger::initialDoorLocations[MAX_DOORS] = {
	Point(79, 2),   // [0] Right Side (Used for Level 1 -> Level 2)
	Point(36, 1),   // [1] Top Side   (Used for Level 1 -> Level 3)
	Point(38, 24),  // [2] Bottom Side (Used for Level 1 -> Final)
	Point(77, 7),    // [3] Switch Door Location (Lvl 1 -> Lvl 4)
	Point(0, 2),    // [4] Left Side (Used for Level 1 <- Level 2)
	Point(36, 24),   // [5] Bottom Side (Used for Level 1 <- Level 3)
	Point(10, 1)    // [6] Upper Side (Lvl 4 -> Lvl 1)
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
			else if (raw == Screen::OBSTACLE) {
				displayChar = raw;
				color = Screen::Color::DarkGray;
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
	bool keepProgramRunning = true;

	while (keepProgramRunning) {
		// 1. Show the menu
		if (!showMenu()) {
			keepProgramRunning = false; // User selected Exit (9)
			break;
		}

		// 2. User chose "Start Game" (1)
		// RESET EVERYTHING BEFORE STARTING
		resetGame();

		// 3. Load the first room
		loadRoom(0);

		// 4. Start the game loop
		gameLoop();

		// When gameLoop returns, the while loop repeats, showing the menu again.
	}
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
	// Reset currentRoom indicator to ensure we know if we loaded a game or not
	currentRoom = -1;

	while (true) {
		screen.loadFromFileToMap("menu.txt");
		screen.draw();
		printMainMenu();

		int choice = NumbersInput();

		switch (choice) {
		case MENU_OPT_START:
		{// Check if files exist
			auto levels = getLevelDefs();
			bool missingFiles = false;
			std::string missingName;

			for (const auto& level : levels) {
				if (!fs::exists(level.filename)) {
					missingFiles = true;
					missingName = level.filename;
					break;
				}
			}

			if (missingFiles) {
				cls();
				int cx = Screen::MAX_X / 2;
				int cy = Screen::MAX_Y / 2;

				if (g_colorsEnabled) setTextColor(Screen::Color::Red);
				gotoxy(cx - 15, cy - 2); std::cout << "ERROR: MISSING FILES";
				if (g_colorsEnabled) setTextColor(Screen::Color::LightGray);

				gotoxy(cx - 20, cy);     std::cout << "Cannot find required map file: " << missingName;
				gotoxy(cx - 20, cy + 2); std::cout << "Game cannot start.";
				gotoxy(cx - 20, cy + 4); std::cout << "Press any key to return to menu...";
				(void)_getch();
				break; // Return to menu loop without starting
			}
		}
		cls();
		return true;

		case MENU_OPT_LOAD:
			showLoadGameMenu();
			// If load successful, currentRoom will be >= 0
			if (currentRoom != -1) {
				return true;
			}
			break;

		case MENU_OPT_INSTRUCTIONS:
			printInstructions();
			break;

		case MENU_OPT_SETTINGS:
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

		case MENU_OPT_GUIDE:
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

void GameManger::printMainMenu() const { // added const
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
	gotoxy(optX, cy + 7); std::cout << "2. Load Game";
	gotoxy(optX, cy + 9); std::cout << "3. Instructions (Rules)";
	gotoxy(optX, cy + 11); std::cout << "4. Choose Color Mode";
	gotoxy(optX, cy + 13); std::cout << "5. Controls Guide";
	gotoxy(optX, cy + 16); std::cout << "9. Exit Game";

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
	gotoxy(leftAlign, startY + 11); std::cout << "5. Turn on Switches -> / To open doors with the number 8";
	gotoxy(leftAlign, startY + 13); std::cout << "6. Watch out for traps -> ! <- you got 3 lives!";



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
	running = true;

	while (running && !won) {

		handleInput();
		if (!running) break;
		updatePlayers();
		

		if (rooms[currentRoom].dark) {
			// Incremental fog redraw
			drawWithFog();
		}
		updateBombs();	
		// HUD 
		printStatsBar();
		Sleep(60);   
	}

	cls();
}


void GameManger::handleInput() {
	if (!_kbhit()) return;

	char key = _getch();

	// Handle Global Keys (ESC)
	if (key == 27) { // ASCII for ESC
		// We are now "paused". Clear HUD area or show pause message.
		gotoxy(0, 0);
		std::cout << "PAUSED: (ESC) Resume | (S) Save Game | (H) Exit to Menu    ";

		while (true) {
			// Wait for a key
			char cmd = _getch();

			if (cmd == 27) {
				// Resume game
				// Redraw stats bar to clear pause message
				printStatsBar();
				break;
			}
			else if (cmd == 'h' || cmd == 'H') {
				running = false; // Exit game loop
				break;
			}
			else if (cmd == 's' || cmd == 'S') {
				askAndSaveGame();
				// After saving, redraw map and stats
				if (rooms[currentRoom].dark) {
					fogInitialized = false; // Force redraw
					drawWithFog();
				}
				else {
					screen.draw();
				}
				printStatsBar();
				break; // Resume game after save
			}
		}
	}
	else {
		// Handle Action Keys
		if (key == P1_DROP_KEY || key == std::toupper(P1_DROP_KEY)) {
			char type = ' ';
			Point p = players[0].dropActiveItem(type);

			if (type == Screen::BOMB && p.getX() != -1) {
				screen.setCell(p.getY(), p.getX(), Screen::BOMB_ACTIVE);
				activeBombs.push_back({ p, 100 });
			}
			printStatsBar();
		}
		else if (key == P2_DROP_KEY || key == std::toupper(P2_DROP_KEY)) {
			char type = ' ';
			Point p = players[1].dropActiveItem(type);

			if (type == Screen::BOMB && p.getX() != -1) {
				screen.setCell(p.getY(), p.getX(), Screen::BOMB_ACTIVE);
				activeBombs.push_back({ p, 100 });
			}
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
			player.move(globalDoors, MAX_DOORS, currentRoom,(&player == &players[0]) ? &players[1] : &players[0],!rooms[currentRoom].dark);

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
		// 3. Trap Logic
		if (player.getTrapState() == true) {
			activeBombs.push_back({ player.getTrapLocation(), 7});
			player.setTrapState(false);
		}

		// 4. Victory
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

void GameManger::resetGame() {
	// 1. Reset Score and Flags and Invertory
	score = 0;
	won = false;
	running = true;
	fogInitialized = false;
	currentRoom = -1;
	Health = 3;
	players[0].removeKeys(Player::getCollectedKeys());
	players[1].removeKeys(Player::getCollectedKeys());
	players[0].setBomb(false);
	players[1].setBomb(false);
	players[0].setTorch(false);
	players[1].setTorch(false);

	// 2. Re-initialize Rooms (Reloads maps from disk/defaults)
	initRooms();

	// 3. Re-initialize Doors (Closes them back up)
	initDoors();


}

void GameManger::initRooms()
{
	// 1. Get definitions
	std::vector<LevelMetadata> levels = getLevelDefs();

	// 2. SORT Lexicographically
	// Order becomes: level1, level2, level3, level4, levelFinal
	std::sort(levels.begin(), levels.end(), [](const LevelMetadata& a, const LevelMetadata& b) {
		return a.filename < b.filename;
		});

	// 3. Initialize Rooms from sorted data
	for (size_t i = 0; i < levels.size() && i < NUMBER_OF_ROOMS; ++i) {
		std::vector<Point> starts = { levels[i].spawnP1, levels[i].spawnP2 };
		rooms[i] = Room(levels[i].filename, starts, levels[i].isDark);
	}
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
	globalDoors[2] = { initialDoorLocations[2], 2, 0, 4, 7, false };

	// Door 4: Switch Door Goes to level 3
	globalDoors[4] = { initialDoorLocations[3], 4, 0, 3, 8, false };


	// ==========================================
	//              LEVEL 2 DOORS
	// ==========================================

	// Door 3: Back to Level 1 (Location: Right Side - SAME AS ENTERING)
	globalDoors[3] = { initialDoorLocations[4], 3, 1, 0, 0, true }; // 0 cost, already open




	// ==========================================
	//              LEVEL 3 DOORS
	// ==========================================

	// Door 5: Back to Level 1 (Location: Top Side - SAME AS ENTERING)
	globalDoors[5] = { initialDoorLocations[5], 5, 2, 0, 0, true }; // 0 cost, already open

	// ==========================================
		//              LEVEL 4 DOORS
    // ==========================================

		// Door 6: Back to Level 1 (Location: Upper part of Level 4)
		// ID: 6
		// Source Room: 4 (Level 4)
		// Target Room: 0 (Level 1)
		// Cost: 0 keys (Open)
	globalDoors[6] = { initialDoorLocations[6], 6, 3, 0, 0, true };
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

		players[0].getPoint().draw(players[0].getChar());
		players[1].getPoint().draw(players[1].getChar());
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

	// This loop consumes any keystrokes stored while the pattern was 
	// playing, ensuring the next _getch() waits for NEW input.
	while (_kbhit()) {
		(void)_getch();
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
	increaseScore(WIN_POINTS,"SIMON SAYS WINS!");
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
		increaseScore(10, "CORRECT ANSWER!");
		Sleep(700);
		// Note: You might want to add score here too
	}
}

void GameManger::increaseScore(int points, const std::string& message)
{
	cls();
	score += points;

	// --- 1. Calculate Dimensions ---
	int cx = Screen::MAX_X / 2;
	int cy = Screen::MAX_Y / 2;

	std::string scoreLine = "SCORE + " + std::to_string(points);

	// Determine the widest content (either the message or the score)
	size_t contentLen = (std::max)(message.length(), scoreLine.length());

	// Ensure the box is at least 20 chars wide for aesthetics
	int minWidth = 22;
	int boxWidth = (std::max)((int)contentLen + 6, minWidth); // +6 for padding (3 spaces each side)

	// Calculate the top-left starting position to center the box
	int startX = cx - (boxWidth / 2);

	// Create the border string dynamically
	std::string border(boxWidth, '#');

	// --- 2. Animation Loop ---
	Screen::Color colors[] = { Screen::Color::Yellow, Screen::Color::LightGreen, Screen::Color::LightCyan };

	for (int i = 0; i < 6; ++i) {
		setTextColor(colors[i % 3]);

		// DRAW: Top Border
		gotoxy(startX, cy - 2);
		std::cout << border;

		// DRAW: Message Line
		gotoxy(startX, cy - 1);
		std::cout << "#";

		// Calculate dynamic padding for Message
		int totalSpacesMsg = boxWidth - 2 - (int)message.length();
		int padLeftMsg = totalSpacesMsg / 2;
		int padRightMsg = totalSpacesMsg - padLeftMsg; // Handles odd numbers safely

		// Print: [Spaces] + [Message] + [Spaces]
		for (int k = 0; k < padLeftMsg; k++) std::cout << " ";
		std::cout << message;
		for (int k = 0; k < padRightMsg; k++) std::cout << " ";
		std::cout << "#";

		// DRAW: Spacer Line (Empty middle row)
		gotoxy(startX, cy);
		std::cout << "#";
		for (int k = 0; k < boxWidth - 2; k++) std::cout << " ";
		std::cout << "#";

		// DRAW: Score Line
		gotoxy(startX, cy + 1);
		std::cout << "#";

		// Calculate dynamic padding for Score
		int totalSpacesScore = boxWidth - 2 - (int)scoreLine.length();
		int padLeftScore = totalSpacesScore / 2;
		int padRightScore = totalSpacesScore - padLeftScore;

		for (int k = 0; k < padLeftScore; k++) std::cout << " ";
		std::cout << scoreLine;
		for (int k = 0; k < padRightScore; k++) std::cout << " ";
		std::cout << "#";

		// DRAW: Bottom Border
		gotoxy(startX, cy + 2);
		std::cout << border;

		if (i < 3) Beep(400 + (i * 100), 50);
		Sleep(150);
	}

	setTextColor(Screen::Color::LightGray);
	Sleep(600);
}

int GameManger::NumbersInput()const
{
	char choice = 0;
	while (true) {
		choice = _getch();
		if (std::isdigit(choice))
			return choice - '0';
	}
} // added const

void GameManger::printStatsBar() const  { // added const
	int totalKeys = Player::getCollectedKeys();

	// Player 1 Status
	std::string inv1 = "P1:";
	if (players[0].hasTorch()) inv1 += " [T] ";
	if (players[0].hasBomb())  inv1 += " [B] ";

	// Player 2 Status
	std::string inv2 = "P2:";
	if (players[1].hasTorch()) inv2 += " [T]";
	if (players[1].hasBomb())  inv2 += " [B]";
	gotoxy(0, 0);
	setTextColor(Screen::Color::Cyan);

	std::cout << "LEVEL: " << (currentRoom + 1)
		<< " | SCORE: " << score
		<< " | HP: " << Health
		<< " | KEYS: " << totalKeys
		<< " | INV: " << inv1 << inv2;

	int currentLen = 75;
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

// bomb functions
void GameManger::updateBombs() {
	if (activeBombs.empty()) return;

	// Use a standard iterator so we can erase items safely
	auto it = activeBombs.begin();
	while (it != activeBombs.end()) {

		// ===========================
		// STATE: FUSE (Ticking down)
		// ===========================
		if (it->state == BombState::FUSE) {
			// 1. Blink Logic 
			int blinkSpeed = (it->timer > 60) ? 15 : (it->timer > 30 ? 8 : 1);

			Screen::Color displayColor = (it->timer / blinkSpeed) % 2 == 0
				? Screen::Color::Red
				: Screen::Color::DarkGray;

			gotoxy(it->position.getX(), it->position.getY());
			setTextColor(displayColor);
			std::cout << Screen::BOMB; // Draw 'B'
			setTextColor(Screen::Color::LightGray);

			// 2. Decrement Fuse
			it->timer--;

			// 3. Transition if Fuse ends
			if (it->timer <= 0) {
				// Move to Ignition phase
				it->state = BombState::IGNITION;
				it->timer = 2; // Run this frame for 2 game cycles (~120ms)

				Beep(600, 50);
			}
			++it;
		}
		// ===========================
		// STATE: ANIMATION SEQUENCES
		// ===========================
		else if (it->state == BombState::IGNITION) {
			drawExplosionFrame(it->position, 1); // Draw White Center
			it->timer--;
			if (it->timer <= 0) {
				it->state = BombState::EXPANSION;
				it->timer = 2; // Next frame duration
				Beep(400, 50);
			}
			++it;
		}
		else if (it->state == BombState::EXPANSION) {
			drawExplosionFrame(it->position, 2); // Draw Red Box
			it->timer--;
			if (it->timer <= 0) {
				it->state = BombState::HEAT;
				it->timer = 2; // Next frame duration
			}
			++it;
		}
		else if (it->state == BombState::HEAT) {
			drawExplosionFrame(it->position, 3); // Draw Yellow Box
			it->timer--;
			if (it->timer <= 0) {
				// Animation done, perform actual destruction logic
				explodeBomb(it->position);
				it->state = BombState::FINISHED;
			}
			++it;
		}
		// ===========================
		// STATE: FINISHED
		// ===========================
		else {
			// Remove from vector
			it = activeBombs.erase(it);
		}
	}
}

void GameManger::drawExplosionFrame(const Point& center, int stage) const { // added const
	int cx = center.getX();
	int cy = center.getY();

	// 1. Setup Style
	char drawChar = ' ';
	Screen::Color color = Screen::Color::LightGray;

	if (stage == 1) {      // IGNITION
		drawChar = static_cast<char>(Screen::BlockType::FullBlock);
		color = Screen::Color::White;
	}
	else if (stage == 2) { // EXPANSION
		drawChar = static_cast<char>(Screen::BlockType::MediumBlock);
		color = Screen::Color::Red;
	}
	else if (stage == 3) { // HEAT
		drawChar = static_cast<char>(Screen::BlockType::LightBlock);
		color = Screen::Color::Yellow;
	}

	setTextColor(color);

	// 2. Draw
	if (stage == 1) {
		gotoxy(cx, cy); 
		std::cout << drawChar;
	}
	else {
		// Stages 2 & 3 (5x5 box)
		for (int y = cy - 2; y <= cy + 2; ++y) {
			for (int x = cx - 4; x <= cx + 4; ++x) {
				// Bounds Check
				if (y < 0 || y > Screen::MAX_Y || x < 0 || x > Screen::MAX_X) continue;

				Point target(x, y);

				// Don't draw ON walls
				if (screen.isWall(target) || screen.isDoor(target) || screen.isSwitchOn(target) || screen.isSwitchOff(target))continue;

				// SHARED LOGIC: Only draw if we have Line of Sight
				if (hasClearPath(center, target)) {
					gotoxy(x, y);
					std::cout << drawChar;
				}
			}
		}
	}
	setTextColor(Screen::Color::LightGray);
}

bool GameManger::hasClearPath(const Point& start, const Point& target) const {
	int x0 = start.getX();
	int y0 = start.getY();
	int x1 = target.getX();
	int y1 = target.getY();

	int dx = std::abs(x1 - x0);
	int dy = -std::abs(y1 - y0);
	int sx = x0 < x1 ? 1 : -1;
	int sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2;

	while (true) {
		// 1. Reached the target? Path is clear.
		if (x0 == x1 && y0 == y1) return true;

		// 2. Check current tile for wall (skip the start point itself)
		if (x0 != start.getX() || y0 != start.getY()) {
			Point p(x0, y0);
			// If we hit a wall or a door, the path is blocked
			if (screen.isWall(p) || screen.isDoor(p)) {
				return false;
			}
		}

		// 3. Move to next tile in the line
		e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; }
		if (e2 <= dx) { err += dx; y0 += sy; }
	}
} // added const

void GameManger::explodeBomb(const Point& center) {
	int cx = center.getX();
	int cy = center.getY();
	bool hit = false;
	// 1. Clear Center Data
	screen.setCell(cy, cx, ' ');

	// 2. Cleanup Visuals (optional, just to be safe)
	drawExplosionFrame(center, 0); // Assuming 0 or a clear helper clears it, 
	// or relies on the next frame redraw.

// 3. Radius Logic (Items/Objects)
	for (int y = cy - 2; y <= cy + 2; ++y) {
		for (int x = cx - 4; x <= cx + 4; ++x) {
			// Bounds check
			if (y < 0 || y > Screen::MAX_Y || x < 0 || x > Screen::MAX_X) continue;

			Point target(x, y);

			// SHARED LOGIC: Physical explosion blocked by wall?
			if (!hasClearPath(center, target)) continue;

			char c = screen.getCharAt(y, x);

			// Destroy objects
			if (c == Screen::OBSTACLE || c == Screen::KEY ||
				c == Screen::TORCH || c == Screen::BOMB || c == Screen::SPRING ||
				c == Screen::RIDDEL || c == Screen::TRAP) {

				screen.setCell(y, x, ' ');
				// Visual update for the map data (not the animation)
				Point(x, y).draw(' ');
			}

			// Handle Player Damage
			for (auto& p : players) {
				int px = p.getX();
				int py = p.getY();

				if (px == x && py == y) {
					// Decrease health
					Health--;
					hit = true;
					printStatsBar();
				}
			}
		}
	}
	// We wait for the bomb destruction then check if player died during explosion
	if (Health <= 0) {
		// Game over logic
		cls();
		gotoxy(Screen::MAX_X / 2 - 5, Screen::MAX_Y / 2);
		std::cout << "GAME OVER!";
		Sleep(2000);
		running = false;
		won = false;
		return;
	}
	else if (hit) {
		// Reset level if not dead
		loadRoom(currentRoom);
		for (auto& pl : players) pl.resetLevelData();
		return;
	}
}

// ===========================
//      Save & Load Logic
// ===========================

void GameManger::askAndSaveGame() {
	cls();
	gotoxy(Screen::MAX_X / 2 - 10, Screen::MAX_Y / 2 - 2);
	std::cout << "=== SAVE GAME ===";
	gotoxy(Screen::MAX_X / 2 - 15, Screen::MAX_Y / 2);
	std::cout << "Enter save name: ";

	std::string name;
	std::cin >> name;

	if (name.empty()) name = "savegame";
	// Append extension if user didn't provide one
	if (name.find(".sav") == std::string::npos) {
		name += ".sav";
	}

	saveGame(name);

	gotoxy(Screen::MAX_X / 2 - 15, Screen::MAX_Y / 2 + 2);
	std::cout << "Game Saved Successfully!";
	Sleep(1000);
}

void GameManger::saveGame(const std::string& filename) {
	std::ofstream file(filename);
	if (!file) return;

	// 1. Commit current room screen to memory before saving
	if (currentRoom >= 0 && currentRoom < NUMBER_OF_ROOMS) {
		screen.saveScreenToRoom(rooms[currentRoom]);
	}

	// 2. Global State
	file << currentRoom << "\n";
	file << score << "\n";
	file << Health << "\n";
	file << Player::collectedKeys << "\n";
	file << Player::AmountOfSwitches << "\n";

	// 3. Player State
	file << NUMBER_OF_PLAYERS << "\n";
	for (int i = 0; i < NUMBER_OF_PLAYERS; ++i) {
		const auto& p = players[i];
		Point pos = p.getPoint();
		file << pos.getX() << " " << pos.getY() << " " << pos.getChar() << "\n"; // Position
		file << p.dir.getDirX() << " " << p.dir.getDirY() << "\n";               // Direction
		file << p.hasTorchFlag << " " << p.hasBombFlag << " " << p.won << " " << p.finishedLevel << "\n";
		file << p.ourRoomIndex << " " << p.targetRoomIndex << "\n";
	}

	// 4. Doors
	file << MAX_DOORS << "\n";
	for (int i = 0; i < MAX_DOORS; ++i) {
		file << globalDoors[i].isOpen << "\n";
	}

	// 5. Rooms (Map state)
	file << NUMBER_OF_ROOMS << "\n";
	for (int i = 0; i < NUMBER_OF_ROOMS; ++i) {
		file << rooms[i].isVisited << "\n";
		if (rooms[i].isVisited) {
			size_t rows = rooms[i].savedMapState.size();
			file << rows << "\n";
			for (const auto& line : rooms[i].savedMapState) {
				file << line << "\n";
			}
		}
	}
}

void GameManger::showLoadGameMenu() {
	cls();
	int cx = Screen::MAX_X / 2;
	int cy = 5;

	gotoxy(cx - 10, cy); std::cout << "=== LOAD GAME ===";

	// List files
	std::vector<std::string> saves;
	try {
		for (const auto& entry : fs::directory_iterator(".")) {
			if (entry.path().extension() == ".sav") {
				saves.push_back(entry.path().string());
			}
		}
	}
	catch (...) {}

	if (saves.empty()) {
		gotoxy(cx - 15, cy + 2); std::cout << "No save files found!";
		Sleep(1500);
		return;
	}

	int row = cy + 2;
	for (size_t i = 0; i < saves.size(); ++i) {
		// Remove "./" prefix for display
		std::string name = saves[i];
		if (name.find(".\\") == 0) name = name.substr(2);

		gotoxy(cx - 15, row++);
		std::cout << (i + 1) << ". " << name;
	}

	gotoxy(cx - 20, row + 1); std::cout << "Enter number to load (0 to cancel): ";
	int choice = NumbersInput();

	if (choice > 0 && choice <= (int)saves.size()) {
		std::string selectedFile = saves[choice - 1];
		if (loadGame(selectedFile)) {
			gotoxy(cx - 10, row + 3); std::cout << "Loaded!";
			Sleep(1000);
		}
		else {
			gotoxy(cx - 10, row + 3); std::cout << "Failed to load!";
			Sleep(1000);
		}
	}
}

bool GameManger::loadGame(const std::string& filename) {
	std::ifstream file(filename);
	if (!file) return false;

	// Temp variables
	int tempVal;

	// 1. Global State
	if (!(file >> currentRoom >> score >> Health >> Player::collectedKeys >> Player::AmountOfSwitches)) return false;

	// 2. Player State
	int numP;
	file >> numP;
	for (int i = 0; i < NUMBER_OF_PLAYERS; ++i) {
		int x, y, dx, dy, roomIdx, tRoom;
		char ch;
		bool hasT, hasB, w, fin;

		file >> x >> y >> ch;
		file >> dx >> dy;
		file >> hasT >> hasB >> w >> fin;
		file >> roomIdx >> tRoom;

		players[i].setPosition(Point(x, y, ch));
		players[i].setDirection(Direction(dx, dy));
		players[i].setTorch(hasT);
		players[i].setBomb(hasB);
		players[i].setWin(w);
		players[i].setFinished(fin);
		players[i].setRoom(roomIdx);
		players[i].targetRoomIndex = tRoom;
	}

	// 3. Doors
	int maxD;
	file >> maxD;
	for (int i = 0; i < MAX_DOORS; ++i) {
		bool open;
		file >> open;
		globalDoors[i].isOpen = open;
	}

	// 4. Rooms
	int numR;
	file >> numR;
	// Handle potential mismatch in number of rooms (just in case code changed)
	int limit = (numR < NUMBER_OF_ROOMS) ? numR : NUMBER_OF_ROOMS;

	for (int i = 0; i < limit; ++i) {
		bool vis;
		file >> vis;
		rooms[i].isVisited = vis;
		rooms[i].savedMapState.clear();

		if (vis) {
			size_t rows;
			file >> rows;
			std::string line;
			// Consume newline after the count
			std::getline(file, line);

			for (size_t r = 0; r < rows; ++r) {
				std::getline(file, line);
				rooms[i].savedMapState.push_back(line);
			}
		}
	}

	// 5. Restore screen
	loadRoom(currentRoom);

	return true;
}