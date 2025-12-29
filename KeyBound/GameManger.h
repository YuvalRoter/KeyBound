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

class GameManger {
private:
    // ===========================
    //       Constants
    // ===========================
    static constexpr char ESC = 27;
    static constexpr std::size_t NUMBER_OF_PLAYERS = 2;
    static constexpr std::size_t NUMBER_OF_ROOMS = 5;
    static const int MAX_DOORS = 7;
    static const int MAX_QUESTIONS = 100;

    // ===========================
    //       Game State
    // ===========================
    bool running = true;
    bool won = false;
    int currentRoom = 0;
    int score = 0;

    // ===========================
    //       Objects
    // ===========================
    Screen screen;

    // Arrays of objects
    Player players[NUMBER_OF_PLAYERS];
    Room rooms[NUMBER_OF_ROOMS];
    Door globalDoors[MAX_DOORS];
    Question questions[MAX_QUESTIONS];

    // ===========================
    //       Fog-of-War State
    // ===========================
    char fogLastFrame[Screen::MAX_Y + 1][Screen::MAX_X + 1]{};
    bool fogInitialized = false;

    int numQuestions = 50;

    // Helper data
    static const Point initialDoorLocations[MAX_DOORS];

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
    GameManger();
    void run();

private:
    // ===========================
    //       Internal Logic
    // ===========================
    void gameLoop();
    void handleInput();
    void updatePlayers();

    // Initialization
    void initRooms();
    void initDoors();
    bool loadQuestionsFromFile(const std::string& filename);
    void resetGame();

    // UI & Rendering
    bool showMenu();
    void printStatsBar();
    void drawWithFog();
    void loadRoom(int index);


    // MENU
    void printMainMenu() ;
    void printInstructions() ;
    void printControls() ;
    void drawSettingsMenu() ; 

    // Riddles & Events
    void handleRiddle(Player& player);
    void handleSimon(Riddle& riddle, Player& player);
    void handleMulti(Riddle& riddle, Player& player);
    Riddle generateRandomRiddle();

    // Utilities
    int NumbersInput();
    void increaseScore(int points, const std::string& message);


    //Bomb
    void updateBombs();
    void explodeBomb(const Point& center);
    void drawExplosionFrame(const Point& center, int stage);

     };