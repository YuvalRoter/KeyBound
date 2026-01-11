#include "StepsRecorder.h"
#include <ctime>
#include <fstream>
#include <iostream>

StepsRecorder::StepsRecorder() {
    // Initialize with a random seed so every session is unique
    randomSeed = (unsigned int)std::time(nullptr);
}

int StepsRecorder::getInput(long gameCycle) {
    if (_kbhit()) {
        int key = _getch();

        // We capture the key press into our history.
        addStep(gameCycle, key);

        return key;
    }
    return 0;
}

void StepsRecorder::handleResult(long gameCycle, ResultType type, const std::string& data) {
    // Log the result event for later verification
    results.push_back({ gameCycle, type, data });
}

bool StepsRecorder::saveResultsFile(const std::string& filename) const {
    std::ofstream outfile(filename);
    if (!outfile) return false;

    // Format: [Count] \n [Time] [Type] [Data] ...
    outfile << results.size() << "\n";
    for (const auto& res : results) {
        outfile << res.time << " " << (int)res.type << " " << res.data << "\n";
    }
    return true;
}