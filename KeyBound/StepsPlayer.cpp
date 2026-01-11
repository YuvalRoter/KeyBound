#include "StepsPlayer.h"
#include <iostream>
#include <fstream>
#include <sstream>

StepsPlayer::StepsPlayer(bool isSilentMode)
    : silent(isSilentMode), resultIndex(0), testFailed(false) {
}

// --- INPUT PLAYBACK ---
int StepsPlayer::getInput(long gameCycle) {
    // popEventAtTime is already implemented in your Steps.cpp!
    // It checks if the current cycle matches the recorded time.
    return popEventAtTime(gameCycle);
}

// --- RESULT VALIDATION ---
void StepsPlayer::handleResult(long gameCycle, Steps::ResultType type, const std::string& data) {
    if (resultIndex < expectedResults.size()) {
        const auto& expected = expectedResults[resultIndex];

        // Check if the current game event matches what was recorded
        if (expected.time == gameCycle && expected.type == type && expected.data == data) {
            resultIndex++; // Match found, move to next expected result
            return;
        }
    }

    // If we reach here, the game performed differently than the recording!
    testFailed = true;
    if (!silent) {
        std::cout << "\n[TEST FAILED] Cycle: " << gameCycle
            << " | Expected Type: " << (int)type
            << " | Received Data: " << data << std::endl;
    }
}

// --- FILE LOADING ---
bool StepsPlayer::loadResultsFile(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile) return false;

    expectedResults.clear();
    int count;
    if (!(infile >> count)) return false;

    for (int i = 0; i < count; ++i) {
        long t;
        int typeInt;
        std::string d;
        if (infile >> t >> typeInt >> d) {
            expectedResults.push_back({ t, static_cast<Steps::ResultType>(typeInt), d });
        }
    }
    return true;
}