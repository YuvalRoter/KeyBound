#pragma once
#include <string>
#include <vector>

enum class ResultType { ScreenChange, LifeLost, Riddle, GameEnd, BombTick };

class Steps {
protected:
    long randomSeed = 0;
public:
    virtual ~Steps() = default;

    // Core Interface
    virtual int getInput(long gameCycle) = 0;
    virtual void handleResult(long gameCycle, ResultType type, const std::string& data) = 0;
    virtual bool isSilent() const = 0;
    virtual long getRandomSeed() const { return randomSeed; }

    // Shared Helpers
    void setRandomSeed(long seed) { randomSeed = seed; }
};