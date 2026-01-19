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
#include "Steps.h"



// ===========================
//      Internal Constants
// ===========================
namespace fs = std::filesystem;
namespace {
	// Random Number Generator setup
	std::mt19937 rng(std::random_device{}());

	// Game Configuration
	constexpr int MAX_SIMON_LENGTH = 4;
	constexpr int MIN_SIMON_LENGTH = 3;
	constexpr int SIMON_DELAY_MS = 400;
	constexpr int FOG_RADIUS_WITH_TORCH = 4;
	constexpr int FOG_RADIUS_NO_TORCH = 0;
	constexpr int WIN_POINTS = 50;
	constexpr int MAX_QUESTIONS = 100;

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

// Helper for random numbers
static int randomInt(int min, int max) {
	std::uniform_int_distribution<int> dist(min, max);
	return dist(rng);
}

// ===========================
//      Constructor
// ===========================
GameManger::GameManger(Steps* handler)
	: stepsHandler(handler)
	, players{
		// Initial dummy positions; actual positions loaded from file in loadLevel()
		Player(Point(1, 1, Screen::PLAYER1), Direction::directions[Direction::STAY], P1_KEYS, screen),
		Player(Point(2, 1, Screen::PLAYER2), Direction::directions[Direction::STAY], P2_KEYS, screen)
	}
{
	rng.seed(stepsHandler->getRandomSeed());

	hideCursor();
	cls();

	
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
	while (running) {
		if (!showMenu()) {
			running = false;
			break;
		}

		if (currentLevelName.empty()) {
			resetGame();
			loadLevel("level1.txt");
		}




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
	currentLevelName = "";
	while (true) {
		screen.loadFromFileToMap("menu.txt");
		screen.draw();
		printMainMenu();
		int choice = NumbersInput();
		if (choice == MENU_OPT_START) return true;
		if (choice == MENU_OPT_LOAD) { showLoadGameMenu(); if (!currentLevelName.empty()) return true; }
		if (choice == MENU_OPT_INSTRUCTIONS) printInstructions();
		if (choice == MENU_OPT_SETTINGS) {
			while (true) {
				drawSettingsMenu();
				int sub = NumbersInput();
				if (sub == 1) g_colorsEnabled = true;
				else if (sub == 2) g_colorsEnabled = false;
				else if (sub == 3) break;
			}
		}
		if (choice == MENU_OPT_GUIDE) printControls();
		if (choice == 9) return false;
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

	while (true) {
		if (stepsHandler->getInput(gameCycle) != 0) break;
		gameCycle++;
		if (!stepsHandler->isSilent()) Sleep(30);
	}

}

void GameManger::gameLoop() {
	running = true;
	while (running && !won) {
		gameCycle++;
		handleInput();
		if (!running) break;

		updatePlayers();
		updateBombs();

		if (!stepsHandler->isSilent()) {
			Room& r = *worldMap[currentLevelName];
			if (r.isDark) drawWithFog();
			printStatsBar();
			if (stepsHandler->isPlayback()) Sleep(20);
			else Sleep(60);
		}
	}
	if (!stepsHandler->isSilent()) cls();
}

void GameManger::handleInput() {
	char key = stepsHandler->getInput(gameCycle);
	if (key == 0) return;

	if (key == 27) {
		gotoxy(0, 0);
		std::cout << "PAUSED: (ESC) Resume | (S) Save | (H) Exit ";
		while (true) {
			char cmd = stepsHandler->getInput(gameCycle);
			if (cmd == 0) { gameCycle++; if (!stepsHandler->isSilent()) Sleep(30); continue; }
			if (cmd == 27) { printStatsBar(); break; }
			if (cmd == 'h' || cmd == 'H') { running = false; break; }
			if (cmd == 's' || cmd == 'S') {
				askAndSaveGame();
				Room& r = *worldMap[currentLevelName];
				if (r.isDark) { fogInitialized = false; drawWithFog(); }
				else screen.draw();
				printStatsBar();
				break;
			}
		}
	}
	else {
		if (key == P1_DROP_KEY || key == std::toupper(P1_DROP_KEY)) {
			char type = ' ';
			Point p = players[0].dropActiveItem(type);
			if (type == Screen::BOMB && p.getX() != -1) {
				screen.setCell(p.getY(), p.getX(), Screen::BOMB_ACTIVE);
				activeBombs.push_back({ p, 100, BombState::FUSE });
			}
			printStatsBar();
		}
		else if (key == P2_DROP_KEY || key == std::toupper(P2_DROP_KEY)) {
			char type = ' ';
			Point p = players[1].dropActiveItem(type);
			if (type == Screen::BOMB && p.getX() != -1) {
				screen.setCell(p.getY(), p.getX(), Screen::BOMB_ACTIVE);
				activeBombs.push_back({ p, 100, BombState::FUSE });
			}
			printStatsBar();
		}
		else {
			for (auto& p : players) p.keyPressed(key);
		}
	}
}

// ===========================
//      Update Logic
// ===========================

void GameManger::updatePlayers() {
	bool allFinished = true;
	std::string nextLevelFile = "";

	Room& currRoom = *worldMap[currentLevelName];
	std::vector<Door> activeDoors;
	for (auto const& [id, door] : currRoom.doors) {
		activeDoors.push_back(door);
	}

	for (auto& player : players) {
		if (!player.isFinished()) {
			player.move(activeDoors.data(), (int)activeDoors.size(),
				0,
				(&player == &players[0]) ? &players[1] : &players[0],
				!currRoom.isDark);

			if (player.getHUD()) {
				player.setHud(false);
				printStatsBar();
			}

			Point p = player.getPoint();
			char c = screen.getCharAt(p.getY(), p.getX());
			if (isdigit(c)) {
				int id = c - '0';
				if (currRoom.doors.count(id)) {
					Door& d = currRoom.doors[id];
					if (Player::getCollectedKeys() >= d.keysRequired) {
						nextLevelFile = d.targetFile;
						player.setFinished(true);
					}
				}
			}
		}

		if (!player.isFinished()) allFinished = false;

		if (player.inRiddle()) {
			player.keyPressed(player.getstaybutton());
			handleRiddle(player);
			printStatsBar();
		}
		if (player.getTrapState()) {
			activeBombs.push_back({ player.getTrapLocation(), 7, BombState::FUSE });
			player.setTrapState(false);
		}
		if (player.hasWon()) {
			won = true;
			cls();
			Sleep(2000);
		}
	}

	if (allFinished && !nextLevelFile.empty()) {
		loadLevel(nextLevelFile);
	}
	else if (allFinished) {
		loadLevel("level1.txt");
	}
}

// ===========================
//      Initialization
// ===========================

void GameManger::loadLevel(const std::string& filename) {
	// 1. Save state of the CURRENT room (if we are leaving one)
	if (!currentLevelName.empty() && worldMap.find(currentLevelName) != worldMap.end()) {
		screen.saveScreenToRoom(*worldMap[currentLevelName]);
	}

	currentLevelName = filename;

	// 2. Ensure Room is Loaded/Cached in WorldMap
	if (worldMap.find(filename) == worldMap.end()) {
		auto newRoom = std::make_unique<Room>();
		newRoom->filename = filename;
		parseLevelFile(filename, *newRoom); // Parse the text file
		worldMap[filename] = std::move(newRoom);
	}

	Room& room = *worldMap[filename];

	// 3. Load Map Data (From memory if visited, else from raw map data)
	if (room.isVisited) {
		screen.loadScreenFromRoom(room);
	}
	else {
		// Construct screen from map data, masking markers (@, &, L)
		for (int y = 0; y <= Screen::MAX_Y; ++y) {
			std::string line = (y < room.mapData.size()) ? room.mapData[y] : "";
			for (int x = 0; x <= Screen::MAX_X; ++x) {
				char c = (x < line.size()) ? line[x] : ' ';
				if (c == '@' || c == '&' || c == 'L') {
					screen.setCell(y, x, ' ');
				}
				else {
					screen.setCell(y, x, c);
				}
			}
			screen.setCell(y, Screen::MAX_X + 1, '\0');
		}

		// Render Door Costs (Visual "Keys Required" Fix)
		// Only needed on first visit. On revisit, 'savedScreen' remembers if they are open/closed.
		for (auto const& [id, door] : room.doors) {
			if (!door.isOpen) { // Only draw if closed
				screen.setCell(door.position.getY(), door.position.getX(), '0' + door.keysRequired);
			}
		}
	}

	// 4. Reset Player Positions
	// We use the dynamic start points parsed from the file
	if (room.p1Start.getX() != -1) players[0].setPosition(room.p1Start);
	if (room.p2Start.getX() != -1) players[1].setPosition(room.p2Start);

	// Reset finished flags for the new level
	for (auto& p : players) p.setFinished(false);

	// 5. Reset fog buffer
	fogInitialized = false;

	// 6. Draw
	if (room.isDark) {
		drawWithFog();
	}
	else {
		screen.draw();
		// Explicitly draw players on top of the map
		players[0].getPoint().draw(players[0].getChar());
		players[1].getPoint().draw(players[1].getChar());
	}

	// 7. Update System
	stepsHandler->handleResult(gameCycle, Steps::ResultType::ScreenChange, filename);
	printStatsBar();
}
 
void GameManger::resetGame() {
	score = 0; won = false; running = true; fogInitialized = false; Health = 3; currentLevelName = "";
	worldMap.clear();
	players[0].resetLevelData(); players[1].resetLevelData();

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
	screen.loadFromFileToMap("riddle1.txt");
	if (r.getType() == RiddleType::MultipleChoice) handleMulti(r, player);
	else handleSimon(r, player);
	screen.restoreBackup();

	Room& rm = *worldMap[currentLevelName];
	if (rm.isDark) { fogInitialized = false; drawWithFog(); }
	else screen.draw();
	player.setInRiddle(false);
}

void GameManger::handleSimon(Riddle& riddle, Player& player) {
	std::vector<int> pattern = riddle.getSimonPattern();
	for (int idx : pattern) {
		screen.drawSimon(-1); Sleep(150);
		screen.drawSimon(idx); Sleep(riddle.getSimonDelayMs());
		screen.drawSimon(-1); Sleep(150);
	}
	while (_kbhit()) (void)_getch();

	for (size_t i = 0; i < pattern.size(); ++i) {
		int digit = NumbersInput();
		screen.drawSimon(digit - 1);
		if (digit - 1 != pattern[i]) {
			cls(); std::cout << "YOU FAILED!"; Sleep(1000); return;
		}
	}
	increaseScore(WIN_POINTS, "SIMON SAYS WINS!");
}

void GameManger::handleMulti(Riddle& riddle, Player& player) {
	cls();
	std::cout << riddle.getQuestion() << "\n";
	const auto& opts = riddle.getOptions();
	for (size_t i = 0; i < opts.size(); ++i) std::cout << (i + 1) << ". " << opts[i] << "\n";

	int choice = NumbersInput() - 1;
	if (choice != riddle.getCorrectIndex()) {
		std::cout << "YOU FAILED!"; Sleep(1000);
	}
	else {
		increaseScore(10, "CORRECT!");
	}
}

Riddle GameManger::generateRandomRiddle() {
	if (randomInt(0, 1) == 0 && numQuestions > 0) {
		int idx = randomInt(0, numQuestions - 1);
		return Riddle::makeMultipleChoice(questions[idx].text, questions[idx].options, questions[idx].correctIndex);
	}
	std::vector<int> p(randomInt(MIN_SIMON_LENGTH, MAX_SIMON_LENGTH));
	for (int& x : p) x = randomInt(0, 3);
	return Riddle::makeSimonSays(p, SIMON_DELAY_MS);
}

void GameManger::increaseScore(int points, const std::string& msg) {
	score += points;
	cls();
	std::cout << msg << "\nSCORE: " << score;
	Sleep(700);
}

int GameManger::NumbersInput() {
	while (true) {
		char c = stepsHandler->getInput(gameCycle);
		if (isdigit(c)) return c - '0';
		gameCycle++;
		if (!stepsHandler->isSilent()) Sleep(30);
	}
}

void GameManger::printStatsBar() const {
	Point loc(0, 0);
	if (worldMap.count(currentLevelName)) {
		loc = worldMap.at(currentLevelName)->legendLoc;
	}

	int lx = loc.getX();
	int ly = loc.getY();

	gotoxy(lx, ly);
	if (g_colorsEnabled) setTextColor(Screen::Color::Cyan);

	std::cout << "LVL:" << currentLevelName.substr(0, 5) << " SC:" << score;
	gotoxy(lx, ly + 1);
	std::cout << "HP:" << Health << " KY:" << Player::getCollectedKeys();

	std::string inv1 = (players[0].hasTorch() ? "T" : " ") + std::string(players[0].hasBomb() ? "B" : " ");
	std::string inv2 = (players[1].hasTorch() ? "T" : " ") + std::string(players[1].hasBomb() ? "B" : " ");

	gotoxy(lx, ly + 2);
	std::cout << "P1:" << inv1 << " P2:" << inv2;

	if (g_colorsEnabled) setTextColor(Screen::Color::LightGray);
}

bool GameManger::loadQuestionsFromFile(const std::string& filename) {
	std::ifstream file(filename);
	if (!file) return false;
	int i = 0;
	while (i < MAX_QUESTIONS && file.peek() != EOF) {
		if (!std::getline(file, questions[i].text)) break;
		for (int j = 0; j < 4; j++) std::getline(file, questions[i].options[j]);
		std::string idx; std::getline(file, idx);
		questions[i].correctIndex = std::stoi(idx);
		std::getline(file, idx); // Expl
		std::getline(file, idx); // Empty
		i++;
	}
	numQuestions = i;
	return true;
}
// bomb functions
void GameManger::updateBombs() {
	if (activeBombs.empty()) return;
	auto it = activeBombs.begin();
	while (it != activeBombs.end()) {
		if (it->state == BombState::FUSE) {
			int blinkSpeed = (it->timer > 60) ? 15 : (it->timer > 30 ? 8 : 1);
			Screen::Color displayColor = (it->timer / blinkSpeed) % 2 == 0
				? Screen::Color::Red : Screen::Color::DarkGray;
			gotoxy(it->position.getX(), it->position.getY());
			setTextColor(displayColor);
			std::cout << Screen::BOMB;
			setTextColor(Screen::Color::LightGray);
			it->timer--;
			if (it->timer <= 0) {
				it->state = BombState::IGNITION;
				it->timer = 2;
				Beep(600, 50);
			}
			++it;
		}
		else if (it->state == BombState::IGNITION) {
			drawExplosionFrame(it->position, 1);
			it->timer--;
			if (it->timer <= 0) { it->state = BombState::EXPANSION; it->timer = 2; Beep(400, 50); }
			++it;
		}
		else if (it->state == BombState::EXPANSION) {
			drawExplosionFrame(it->position, 2);
			it->timer--;
			if (it->timer <= 0) { it->state = BombState::HEAT; it->timer = 2; }
			++it;
		}
		else if (it->state == BombState::HEAT) {
			drawExplosionFrame(it->position, 3);
			it->timer--;
			if (it->timer <= 0) {
				explodeBomb(it->position);
				it->state = BombState::FINISHED;
			}
			++it;
		}
		else {
			it = activeBombs.erase(it);
		}
	}
}

void GameManger::drawExplosionFrame(const Point& center, int stage) const {
	int cx = center.getX();
	int cy = center.getY();
	char drawChar = ' ';
	Screen::Color color = Screen::Color::LightGray;

	if (stage == 1) { drawChar = (char)Screen::BlockType::FullBlock; color = Screen::Color::White; }
	else if (stage == 2) { drawChar = (char)Screen::BlockType::MediumBlock; color = Screen::Color::Red; }
	else if (stage == 3) { drawChar = (char)Screen::BlockType::LightBlock; color = Screen::Color::Yellow; }

	setTextColor(color);
	if (stage == 1) {
		gotoxy(cx, cy); std::cout << drawChar;
	}
	else {
		for (int y = cy - 2; y <= cy + 2; ++y) {
			for (int x = cx - 4; x <= cx + 4; ++x) {
				if (y < 0 || y > Screen::MAX_Y || x < 0 || x > Screen::MAX_X) continue;
				Point target(x, y);
				if (screen.isWall(target) || screen.isDoor(target)) continue;
				if (hasClearPath(center, target)) {
					gotoxy(x, y); std::cout << drawChar;
				}
			}
		}
	}
	setTextColor(Screen::Color::LightGray);
}

bool GameManger::hasClearPath(const Point& start, const Point& target) const {
	int x0 = start.getX(); int y0 = start.getY();
	int x1 = target.getX(); int y1 = target.getY();
	int dx = std::abs(x1 - x0); int dy = -std::abs(y1 - y0);
	int sx = x0 < x1 ? 1 : -1; int sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2;

	while (true) {
		if (x0 == x1 && y0 == y1) return true;
		if (x0 != start.getX() || y0 != start.getY()) {
			Point p(x0, y0);
			if (screen.isWall(p) || screen.isDoor(p)) return false;
		}
		e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; }
		if (e2 <= dx) { err += dx; y0 += sy; }
	}
}
void GameManger::explodeBomb(const Point& center) {
	int cx = center.getX(); int cy = center.getY();
	screen.setCell(cy, cx, ' ');
	bool hit = false;
	for (int y = cy - 2; y <= cy + 2; ++y) {
		for (int x = cx - 4; x <= cx + 4; ++x) {
			if (y < 0 || y > Screen::MAX_Y || x < 0 || x > Screen::MAX_X) continue;
			Point target(x, y);
			if (!hasClearPath(center, target)) continue;
			char c = screen.getCharAt(y, x);
			if (c == Screen::OBSTACLE || c == Screen::KEY || c == Screen::TORCH || c == Screen::BOMB || c == Screen::SPRING || c == Screen::RIDDEL || c == Screen::TRAP) {
				screen.setCell(y, x, ' ');
			}
			for (auto& p : players) {
				if (p.getX() == x && p.getY() == y) {
					Health--;
					hit = true;
					printStatsBar();
					stepsHandler->handleResult(gameCycle, Steps::ResultType::LifeLost, std::to_string(Health));
				}
			}
		}
	}
	if (Health <= 0) {
		cls();
		std::cout << "GAME OVER!";
		Sleep(2000);
		running = false;
		won = false;
		stepsHandler->handleResult(gameCycle, Steps::ResultType::GameEnd, std::to_string(score));
	}
	else if (hit) {
		loadLevel(currentLevelName);
		for (auto& pl : players) pl.resetLevelData();
	}
}
// ===========================
//      Save / Load
// ===========================

void GameManger::askAndSaveGame() {
	cls();
	std::cout << "Saving... Enter Name: ";
	std::string name; std::cin >> name;
	if (name.find(".sav") == std::string::npos) name += ".sav";
	saveGame(name);
}

void GameManger::saveGame(const std::string& filename) {
	std::ofstream file(filename);
	if (!file) return;

	file << currentLevelName << "\n";
	file << score << " " << Health << " " << Player::getCollectedKeys() << "\n";

	file << 2 << "\n"; // Num Players
	for (const auto& p : players) {
		Point pos = p.getPoint();
		file << pos.getX() << " " << pos.getY() << " " << p.getChar() << "\n";
		// Need access to Direction dx/dy via getters in Player or store them
		// Simplified for now:
		file << 0 << " " << 0 << "\n";
		file << p.hasTorch() << " " << p.hasBomb() << " " << p.hasWon() << " " << p.isFinished() << "\n";
		file << -1 << " " << -1 << "\n";
	}

	// Minimal save for dynamic map: just the level name is often enough if we don't track opened doors persistently across sessions
}

bool GameManger::loadGame(const std::string& filename) {
	std::ifstream file(filename);
	if (!file) return false;

	file >> currentLevelName;
	int k, dummy;
	file >> score >> Health >> k;
	// Trick: we can't set static keys easily unless we loop k times, or just trust logic

	int numP; file >> numP;
	for (int i = 0; i < 2; ++i) {
		int x, y, dx, dy; char ch; bool t, b, w, f;
		file >> x >> y >> ch;
		file >> dx >> dy;
		file >> t >> b >> w >> f;
		file >> dummy >> dummy;

		players[i].setPosition(Point(x, y, ch));
		players[i].setTorch(t);
		players[i].setBomb(b);
		players[i].setWin(w);
		players[i].setFinished(f);
	}
	loadLevel(currentLevelName);
	return true;
}

void GameManger::showLoadGameMenu() {
	cls();
	std::cout << "Enter filename to load: ";
	std::string name; std::cin >> name;
	if (loadGame(name)) {
		std::cout << "Loaded!";
	}
	else {
		currentLevelName = "";
	}
}

void GameManger::parseLevelFile(const std::string& filename, Room& room) {
	std::ifstream file(filename);
	if (!file) return;

	std::string line;
	int y = 0;
	std::vector<std::string> allLines;

	while (std::getline(file, line)) {
		allLines.push_back(line);
	}

	bool parsingMap = true;
	for (size_t i = 0; i < allLines.size(); ++i) {
		std::string& currentLine = allLines[i];

		// Check for Properties
		if (currentLine.rfind("DARK", 0) == 0) {
			room.isDark = true;
			parsingMap = false;
			continue;
		}
		if (currentLine.rfind("DOOR", 0) == 0) {
			// Format: DOOR <id_char> <file> <keys>
			std::stringstream ss(currentLine);
			std::string cmd, target;
			char idChar;
			int cost;
			ss >> cmd >> idChar >> target >> cost;

			// Find char in mapData
			Point doorPos(-1, -1);
			for (int r = 0; r < room.mapData.size(); r++) {
				size_t c = room.mapData[r].find(idChar);
				if (c != std::string::npos) {
					doorPos = Point((int)c, r);
					break;
				}
			}

			if (doorPos.getX() != -1) {
				int idVal = idChar - '0';
				room.doors[idVal] = Door(idVal, doorPos, target, cost);
			}
			parsingMap = false;
			continue;
		}

		if (parsingMap && y <= Screen::MAX_Y) {
			for (int x = 0; x < currentLine.length(); x++) {
				char c = currentLine[x];
				if (c == '@') room.p1Start = Point(x, y);
				else if (c == '&') room.p2Start = Point(x, y);
				else if (c == 'L') room.legendLoc = Point(x, y);
			}
			room.mapData.push_back(currentLine);
			y++;
		}
	}
}