#include "StepsPlayer.h"
#include <iostream>
#include <fstream>
#include <sstream>

StepsPlayer::StepsPlayer(bool isSilentMode)
    : silent(isSilentMode), resultIndex(0), testFailed(false) {
}

// --- INPUT PLAYBACK ---
int StepsPlayer::getInput(long gameCycle) {
    // 1. Handling Test Failure (Stop the Game)
   
    static int exitSequenceStep = 0;

    if (testFailed) {
        if (exitSequenceStep == 0) {
            exitSequenceStep++;
            return 27; // Step 1: Send ESC (Pause Game / Close Dialogs)
        }
        else if (exitSequenceStep == 1) {
            exitSequenceStep++;
            return 'h'; // Step 2: Send 'h' (Exit Gameplay to Main Menu)
        }
        else {
            return '9'; // Step 3: Send '9' (Select "Exit Game" in Main Menu)
        }
    }

    // 2. Timeout Check (Fix for "results doesn't work")
    // If the game time has passed the time of the next expected result,
    // it means the event didn't happen when it should have. Fail immediately.
    if (resultIndex < expectedResults.size()) {
        const auto& expected = expectedResults[resultIndex];

        // If we missed the timestamp of an event
        if (gameCycle > expected.time) {
            testFailed = true;
            if (!silent) {
                // We print to cerr to ensure it's not buffered or lost easily
                std::cerr << "\n[TEST FAILED] Timeout / Missed Event!"
                    << "\n              Expected Event at Cycle: " << expected.time
                    << "\n              Current Cycle: " << gameCycle
                    << "\n              Type: " << (int)expected.type
                    << " Data: " << expected.data << std::endl;
            }
            // Trigger exit sequence immediately
            exitSequenceStep = 1; // Start at 1 because we want to return 27 now
            return 27;
        }
    }

    // 3. Normal Playback
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

        // Detailed debug info if available
        if (resultIndex < expectedResults.size()) {
            std::cout << "              (Expected: Time=" << expectedResults[resultIndex].time
                << " Type=" << (int)expectedResults[resultIndex].type
                << " Data='" << expectedResults[resultIndex].data << "')\n";
        }
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

        // Read Time and Type
        if (infile >> t >> typeInt) {
            // 1. Consume the single space separator between Type and Data
            char separator;
            infile.get(separator);

            // 2. Read the rest of the line as the Data string
            std::getline(infile, d);

            // 3. Remove carriage return '\r' if present (Windows/Unix compatibility)
            if (!d.empty() && d.back() == '\r') {
                d.pop_back();
            }

            expectedResults.push_back({ t, static_cast<Steps::ResultType>(typeInt), d });
        }
    }
    return true;
}