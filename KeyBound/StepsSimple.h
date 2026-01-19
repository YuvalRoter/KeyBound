#pragma once
#pragma once
#include "Steps.h"
#include <conio.h> // for _kbhit and _getch

class StepsSimple : public Steps {
public:
    // This makes the game play like Ex2
    int getInput(long gameCycle) override {
        if (_kbhit()) {
            return _getch();
        }
        return 0;
    }

    // You want to see the game while testing
    bool isSilent() const override { return false; }

    // For testing Person A logic, this does nothing
    void handleResult(long gameCycle, ResultType type, const std::string& data) override {
        // Optional: print to console just to see if your hooks work
        // std::cout << "Event at " << gameCycle << ": " << data << std::endl;
    }
};