#pragma once
#include "Steps.h"
#include <vector>
#include <string>

// Class responsible for recording game inputs and events to files
class StepsRecorder : public Steps {
private:
    // Internal structure to hold recorded results before saving
    struct RecordedResult {
        long time;
        ResultType type;
        std::string data;
    };

    std::vector<RecordedResult> results;

public:
    StepsRecorder();

    // ===========================
    //    Steps Interface Override
    // ===========================

    // Captures real user input from keyboard using _kbhit/_getch
    // and records it to the steps list.
    virtual int getInput(long gameCycle) override;

    // Records game events (ScreenChange, LifeLost, etc.) to the results list.
    virtual void handleResult(long gameCycle, ResultType type, const std::string& data) override;

    // Recorder is never silent (it's interactive or observing).
    virtual bool isSilent() const override { return false; }

    // ===========================
    //       File I/O
    // ===========================

    // Helper to save both the .steps file (via base class) and .result file
    void saveAllFiles(const std::string& stepsFilename, const std::string& resultFilename);

private:
    // Saves the 'results' vector to the specified text file
    bool saveResults(const std::string& filename) const;
};