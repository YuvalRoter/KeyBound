#include "GameManger.h"
#include "StepsSimple.h"
#include "StepsPlayer.h"
#include "StepsRecorder.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    Steps* handler = nullptr;
    bool isRecording = false;
    bool isLoading = false;

    // --- Argument Parsing ---
    if (argc > 1) {
        std::string arg = argv[1];

        if (arg == "-load") {
            // LOAD MODE: Playback from file
            bool silent = (argc > 2 && std::string(argv[2]) == "-silent");
            auto player = new StepsPlayer(silent);
   

            // Load the files
            if (!player->loadFile("adv-world.steps")) {
                std::cerr << "Error: Could not load adv-world.steps\n";
                delete player;
                return 1;
            }
            if (!player->loadResultsFile("adv-world.result")) {
                std::cerr << "Error: Could not load adv-world.result\n";
                delete player;
                return 1;
            }
            handler = player;
        }
        else if (arg == "-save") {
            // SAVE MODE: Record user input
            handler = new StepsRecorder();
            isRecording = true;
        }
    }

    // DEFAULT MODE: Simple gameplay (no recording/playback)
    if (handler == nullptr) {
        handler = new StepsSimple();
    }

    // --- Run Game ---
    GameManger game(handler);
    game.run();

    // --- Save if Recording ---
    if (isRecording) {
        // We know handler is a StepsRecorder because we set isRecording=true
        StepsRecorder* recorder = static_cast<StepsRecorder*>(handler);
        recorder->saveAllFiles("adv-world.steps", "adv-world.result");
    }
    else if (argc > 1 && std::string(argv[1]) == "-load") {
        // Optional: Print final test status
        StepsPlayer* player = static_cast<StepsPlayer*>(handler);
        if (player->hasTestPassed()) {
            std::cout << "\n>>> TEST PASSED <<<\n";
        }
        else {
            std::cout << "\n>>> TEST FAILED <<<\n";
        }
    }

    delete handler;
    return 0;
}