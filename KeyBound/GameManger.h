#pragma once

#include "Screen.h"
#include "Player.h"
#include <string>
#include <iostream>   
#include <cstddef>
#include <filesystem>
#include "Riddle.h"
#include "Room.h"
#include "Question.h"


class GameManger {
    static constexpr char ESC = 27;
    static constexpr char EXIT = '9';
    static constexpr std::size_t NUMBER_OF_PLAYERS = 2;
    static constexpr std::size_t NUMBER_OF_ROOMS = 3;
    Screen  screen;
    Player  players[NUMBER_OF_PLAYERS];
    Room    rooms[NUMBER_OF_ROOMS]; 
    int     currentRoom = 0;       
    bool    running = true;
    bool    won = false;


    int score = 0;

    static const int MAX_DOORS = 9;
    Door globalDoors[MAX_DOORS];
    static const int MAX_QUESTIONS = 100;
    Question questions[MAX_QUESTIONS];
    int numQuestions = 50;   // how many were actually loaded from file

public:
    GameManger();

    void run();

private:


    void printStatsBar();

    void initDoors();

	bool showMenu();    // returns false if user chose EXIT

    void loadMap(const std::string& filename) {
            if (!screen.loadFromFileToMap(filename))  // failed to load
                std::cout << "Failed to load map!\n";
            screen.draw();
    }
    void gameLoop();      // main game loop

    // helpers
    void updatePlayers(); // movement, win, riddles

    void handleInput();   // read keyboard, move players / pause

    void handleRiddle(Player& player); 

    void handleSimon(Riddle& riddle, Player& player);

    void handleMulti(Riddle& riddle,Player& player); 

    Riddle generateRandomRiddle();
    int NumbersInput();
    void increaseScore(int points);

    void initRooms();            // fill rooms[]
    void loadRoom(int index);    // load room & position players
    bool loadQuestionsFromFile(const std::string& filename);

    void drawWithFog();
};
