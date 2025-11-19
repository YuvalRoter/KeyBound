#pragma once

#include "Screen.h"
#include "Player.h"
#include <string>
#include <iostream>   
#include <cstddef>
#include <filesystem>
#include "Riddle.h"


class GameManger {
    static constexpr char ESC = 27;
    static constexpr char EXIT = '9';
    static constexpr std::size_t NUMBER_OF_PLAYERS = 2;
    Screen  screen;
    Player  players[NUMBER_OF_PLAYERS];
    bool    running = true;
    bool    won = false;
public:
    GameManger();

    void run();

private:
    
	bool showMenu();    // returns false if user chose EXIT

    void loadMap(const std::string& filename) {
            if (!screen.loadFromFile(filename))  // failed to load
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

    static Riddle generateRandomRiddle();
    int NumbersInput();

};
