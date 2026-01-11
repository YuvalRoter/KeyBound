#pragma once
#include "Steps.h"
#include <vector>
#include <string>
#include <conio.h> 

// Structure to hold results (screen changes, life lost, etc.)
struct RecordedResult {
    long time;
    Steps::ResultType type;
    std::string data;
};

class StepsRecorder : public Steps {
private:
    std::vector<RecordedResult> results;

public:
    StepsRecorder();

    // --- INPUT ---
 
    virtual int getInput(long gameCycle) override;

    // --- RESULTS ---.
    virtual void handleResult(long gameCycle, ResultType type, const std::string& data) override;

    // --- CONFIG ---
    // We are playing manually, so we want to see the screen (Not silent).
    virtual bool isSilent() const override { return false; }

    // --- FILE I/O ---
    // Saves the 'results' vector to disk. 
    bool saveResultsFile(const std::string& filename) const;
};