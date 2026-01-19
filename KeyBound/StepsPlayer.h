#pragma once
#include "Steps.h"
#include <vector>
#include <string>

// Structure to hold expected results from adv-world.result
struct ExpectedResult {
    long time;
    Steps::ResultType type;
    std::string data;
};

class StepsPlayer : public Steps {
private:
    std::vector<ExpectedResult> expectedResults;
    size_t resultIndex = 0;
    bool silent;
    bool testFailed = false;

public:
    // Constructor: receives whether we are in -silent mode
    StepsPlayer(bool isSilentMode);

    // Overrides from Steps base class
    virtual int getInput(long gameCycle) override;
    virtual void handleResult(long gameCycle, Steps::ResultType type, const std::string& data) override;
    virtual bool isSilent() const override { return silent; }
    virtual bool isPlayback() const override { return true; }
    // New methods for Loading
    bool loadResultsFile(const std::string& filename);
    bool hasTestPassed() const { return !testFailed; }
};