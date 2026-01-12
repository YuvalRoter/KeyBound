#include "StepsRecorder.h"
#include <conio.h>  // For _kbhit(), _getch()
#include <iostream>
#include <fstream>
#include <ctime>    // For time()
#include <cstdlib>  // For rand(), srand()

StepsRecorder::StepsRecorder() {
    // Generate a random seed based on current time for this recording session.
    // This seed will be stored in the base 'Steps' class and saved to the file,
    // ensuring that playback uses the exact same random sequence.
    unsigned int seed = static_cast<unsigned int>(std::time(nullptr));
    setRandomSeed(seed);
}

int StepsRecorder::getInput(long gameCycle) {
    // Check if a key is pressed (non-blocking)
    if (_kbhit()) {
        int ch = _getch();

        addStep(gameCycle, ch);

        return ch;
    }
    return 0;
}

void StepsRecorder::handleResult(long gameCycle, ResultType type, const std::string& data) {
    // Store the event for validation files
    results.push_back({ gameCycle, type, data });
}

void StepsRecorder::saveAllFiles(const std::string& stepsFilename, const std::string& resultFilename) {
    // 1. Save Steps
    // The base class 'Steps::saveFile' is expected to handle the serialization 
    // of the random seed, map file list, and the recorded input steps.
    if (saveFile(stepsFilename)) {
        std::cout << "Recorded steps successfully saved to " << stepsFilename << std::endl;
    }
    else {
        std::cerr << "Error: Failed to save steps file!" << std::endl;
    }

    // 2. Save Results
    if (saveResults(resultFilename)) {
        std::cout << "Expected results successfully saved to " << resultFilename << std::endl;
    }
    else {
        std::cerr << "Error: Failed to save results file!" << std::endl;
    }
}

bool StepsRecorder::saveResults(const std::string& filename) const {
    std::ofstream outfile(filename);
    if (!outfile) return false;

    // File Format:
    // [Count]
    // [Time] [Type] [Data]
    // ...

    outfile << results.size() << "\n";

    for (const auto& res : results) {
        outfile << res.time << " "
            << static_cast<int>(res.type) << " "
            << res.data << "\n";
    }

    outfile.close();
    return true;
}