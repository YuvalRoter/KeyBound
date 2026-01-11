#include "StepsRecorder.h"
#include <conio.h> 
#include <fstream>
#include <iostream>
#include <ctime>

StepsRecorder::StepsRecorder(bool enableSave) : saveToFile(enableSave) {
    // Generate a random seed for this session
    // In Save/Simple mode, we create the randomness.
    setRandomSeed(static_cast<long>(std::time(nullptr)));
}

int StepsRecorder::getInput(long gameCycle) {
    if (_kbhit()) {
        int ch = _getch();

        // If we are saving, record this input event
        if (saveToFile) {
            steps.push_back({ gameCycle, ch });
        }
        return ch;
    }
    return 0;
}

void StepsRecorder::handleResult(long gameCycle, ResultType type, const std::string& data) {
    if (saveToFile) {
        results.push_back({ gameCycle, type, data });
    }
}

void StepsRecorder::saveFiles() {
    if (!saveToFile) return;

    // 1. Save Steps
    std::ofstream stepFile("adv-world.steps");
    if (stepFile.is_open()) {
        stepFile << randomSeed << "\n";

        stepFile << mapFiles.size() << "\n";
        for (const auto& map : mapFiles) {
            stepFile << map << " ";
        }
        stepFile << "\n";

        stepFile << steps.size() << "\n";
        for (const auto& s : steps) {
            stepFile << s.time << " " << s.input << "\n";
        }
        stepFile.close();
        std::cout << "Game saved to adv-world.steps" << std::endl;
    }

    // 2. Save Results
    std::ofstream resFile("adv-world.result");
    if (resFile.is_open()) {
        resFile << results.size() << "\n";
        for (const auto& r : results) {
            resFile << r.time << " " << static_cast<int>(r.type) << " " << r.data << "\n";
        }
        resFile.close();
        std::cout << "Results saved to adv-world.result" << std::endl;
    }
}