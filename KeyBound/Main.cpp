#include "GameManger.h"
#include "StepsSimple.h"
#include "StepsPlayer.h"
#include "StepsRecorder.h"

int main(int argc, char* argv[]) {
    Steps* handler = nullptr;
    bool shouldSave = false; // Flag to determine if we write files at the end
    // Filenames as per specification
    const std::string stepsFile = "adv-world.steps";
    const std::string resultFile = "adv-world.result";
    // --- ARGUMENT PARSING ---
    if (argc > 1) {
        std::string arg = argv[1];

        // 1. Load Mode (Playback)
        if (arg == "-load") {
            bool silent = (argc > 2 && std::string(argv[2]) == "-silent");
            StepsPlayer* player = new StepsPlayer(silent);

            if (!player->loadFile(stepsFile)) {
                std::cerr << "Error: Could not load " << stepsFile << std::endl;
                delete player; return 1;
            }
            if (!player->loadResultsFile(resultFile)) {
                std::cerr << "Error: Could not load " << resultFile << std::endl;
                delete player; return 1;
            }
            handler = player;
        }
        // 2. Save Mode (Recording)
        else if (arg == "-save") {
            handler = new StepsRecorder();
            shouldSave = true; // Enable saving at exit
        }
    }

    // --- DEFAULT MODE ---
    // If no specific handler was created, we use the Recorder as our "Simple" player.
    // It works exactly like the manual player, but keeps logs in memory.
    if (handler == nullptr) {
        handler = new StepsRecorder();
        shouldSave = false; // We will discard the logs at the end
    }

    // --- GAME LOOP ---
    GameManger game(handler);
    game.run();

    // --- POST-GAME LOGIC ---
    // Only save if the user explicitly requested it via -save
    if (shouldSave) {
        StepsRecorder* recorder = dynamic_cast<StepsRecorder*>(handler);
        if (recorder) {
            std::cout << "\nSaving recording..." << std::endl;

            // Save inputs (Steps.cpp implementation)
            if (recorder->saveFile(stepsFile)) {
                std::cout << "Steps saved to " << stepsFile << std::endl;
            }
            else {
                std::cerr << "Failed to save " << stepsFile << std::endl;
            }

            // Save results (StepsRecorder.cpp implementation)
            if (recorder->saveResultsFile(resultFile)) {
                std::cout << "Results saved to " << resultFile << std::endl;
            }
            else {
                std::cerr << "Failed to save " << resultFile << std::endl;
            }
        }
    }
    // Logic for Playback verification
    else if (dynamic_cast<StepsPlayer*>(handler)) {
        StepsPlayer* player = dynamic_cast<StepsPlayer*>(handler);
        if (player->hasTestPassed()) {
            std::cout << "\n[TEST PASSED] Gameplay matched the recording." << std::endl;
        }
        else {
            std::cout << "\n[TEST FAILED] Gameplay deviated from recording." << std::endl;
        }
    }

    delete handler;
    return 0;
}