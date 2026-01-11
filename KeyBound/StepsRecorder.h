#pragma once
#include "Steps.h"
#include <vector>


struct RecordedStep {
    long time;
    int input;
};

struct RecordedResult {
    long time;
    ResultType type;
    std::string data;
};

class StepsRecorder : public Steps {
private:
    std::vector<RecordedStep> steps;
    std::vector<RecordedResult> results;
    bool saveToFile;

public:
    // If saveToFile is false, it acts as a normal "Simple" game helper
    StepsRecorder(bool enableSave);
    
    // Captures real keyboard input using _kbhit() / _getch()
    int getInput(long gameCycle) override;

    // Stores the result in memory
    void handleResult(long gameCycle, ResultType type, const std::string& data) override;

    // Recorder is never silent (it's interactive)
    bool isSilent() const override { return false; }

    // Writes adv-world.steps and adv-world.result
    void saveFiles();
};