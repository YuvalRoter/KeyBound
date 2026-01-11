#include "StepsPlayer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm> 
#include <cctype>

StepsPlayer::StepsPlayer(bool isSilentMode)
    : silent(isSilentMode), resultIndex(0), testFailed(false) {
}

// --- INPUT PLAYBACK ---
int StepsPlayer::getInput(long gameCycle) {
    // Delegates to the base class to retrieve the step recorded for this time
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

        if (resultIndex < expectedResults.size()) {
            std::cout << "             Expected: " << expectedResults[resultIndex].data << std::endl;
        }
    }
}

// --- FILE LOADING ---
bool StepsPlayer::loadResultsFile(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile) return false;

    // Clear previous data
    expectedResults.clear();

    int count;
    if (!(infile >> count)) return false;

    for (int i = 0; i < count; ++i) {
        long t;
        int typeInt;
        std::string d;

        if (!(infile >> t >> typeInt)) break;

        char c = infile.get(); // consume space
        std::getline(infile, d);

        if (!d.empty() && d.back() == '\r') {
            d.pop_back();
        }

        Steps::ResultType typeEnum = static_cast<Steps::ResultType>(typeInt);
        expectedResults.push_back({ t, typeEnum, d });

        // PARSE & ADD INPUTS FROM RESULT FILE
        if (typeEnum == Steps::ResultType::Input && !d.empty()) {
            char key = d.back();
            if (std::isalpha(key)) {
                key = std::tolower(key);
            }
            addStep(t, key);
        }
    }

    // SORT & DEDUPLICATE
    // Ensure events are in order and we don't have duplicates from loading both files
    std::sort(recordedSteps.begin(), recordedSteps.end(), [](const Step& a, const Step& b) {
        return a.time < b.time;
        });

    auto last = std::unique(recordedSteps.begin(), recordedSteps.end(), [](const Step& a, const Step& b) {
        return a.time == b.time && a.input == b.input;
        });
    recordedSteps.erase(last, recordedSteps.end());

    return true;
}