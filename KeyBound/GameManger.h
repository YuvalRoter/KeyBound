#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "Screen.h"
#include "Player.h"
#include "Riddle.h"
#include "Room.h"
#include "Door.h"       
#include "Question.h"  
class Steps;

class GameManger {
private:

    long gameCycle = 0;
    Steps* stepsHandler = nullptr;

    // ===========================
    //       Game State
    // ===========================
    bool running = true;
    bool won = false;
    int score = 0;
    int Health = 3;

    // Dynamic World Management
    std::string currentLevelName;
    std::map<std::string, std::unique_ptr<Room>> worldMap;

    // ===========================
    //       Objects
    // ===========================
    Screen screen;
    Player players[2];
    Question questions[100];
    int numQuestions = 0;


    // ===========================
    //       Fog-of-War State
    // ===========================
    char fogLastFrame[Screen::MAX_Y + 1][Screen::MAX_X + 1]{};
    bool fogInitialized = false;


    // Helper data
    void parseLevelFile(const std::string& filename, Room& room);

    // ===========================
    //       Bomb Struct
    // ===========================

    enum class BombState {
        FUSE,       // Ticking down (blinking 'B')
        IGNITION,   // Center white block
        EXPANSION,  // Red expanding box
        HEAT,       // Yellow heat box
        FINISHED    // Ready to be removed
    };

    struct ActiveBomb {
        Point position;
        int timer;        // Used for both fuse countdown AND animation frame duration
        BombState state;  // Current stage of the bomb
    };

    std::vector<ActiveBomb> activeBombs;

public:
    // ===========================
    //       Public API
    // ===========================
    GameManger(Steps* handler);
    void run();

private:
    // ===========================
    //       Internal Logic
    // ===========================
    void gameLoop();
    void handleInput();
    void updatePlayers();

    // Initialization
    void loadLevel(const std::string& filename);
    bool loadQuestionsFromFile(const std::string& filename);
    void resetGame();

    // UI & Rendering
    bool showMenu();
    void printStatsBar() const;
    void drawWithFog();


    // MENU
    void printMainMenu() const; 
    void printInstructions() ;
    void printControls() ;
    void drawSettingsMenu() ; 

    // Riddles & Events
    void handleRiddle(Player& player);
    void handleSimon(Riddle& riddle, Player& player);
    void handleMulti(Riddle& riddle, Player& player);
    Riddle generateRandomRiddle();

    // Utilities
    int NumbersInput() ;  
    void increaseScore(int points, const std::string& message);
    Door* findDoorAt(Point p);


    // Save & Load
    void saveGame(const std::string& filename);
    bool loadGame(const std::string& filename);
    void showLoadGameMenu();
    void askAndSaveGame();

    //Bomb
    void updateBombs();
    void explodeBomb(const Point& center);
    void drawExplosionFrame(const Point& center, int stage) const;
    bool hasClearPath(const Point& start, const Point& target) const;
     };