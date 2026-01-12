#pragma once
#include <string>
#include <vector>
#include <fstream>

// A single recorded event
struct Step {
    long time;   // The game cycle (iteration number) when the input occurred
    int input;   // The character/key code pressed (int to handle special keys if needed)
};

class Steps {
protected:
    unsigned int randomSeed;
    std::vector<std::string> mapFiles; // To verify we are playing the correct level set
    std::vector<Step> recordedSteps;

    // Iterator index for O(1) access during playback
    size_t playbackIndex = 0;
 
public:
    virtual bool isSilent() const = 0;

    Steps() : randomSeed(0), playbackIndex(0) {}

    enum class ResultType { ScreenChange, LifeLost, Riddle, GameEnd, BombTick };
    virtual int getInput(long gameCycle) = 0;
    virtual void handleResult(long gameCycle, ResultType type, const std::string& data) = 0;

    // ===========================
    //       Configuration
    // ===========================
    void setRandomSeed(unsigned int seed) { randomSeed = seed; }
    unsigned int getRandomSeed() const { return randomSeed; }
    void setMapFiles(const std::vector<std::string>& files) { mapFiles = files; }
    const std::vector<std::string>& getMapFiles() const { return mapFiles; }

    // ===========================
    //       Recording API
    // ===========================
    // Call this in -save mode whenever the user presses a key
    void addStep(long time, int data) {
        recordedSteps.push_back({ time, data });
    }

    // ===========================
    //       Playback API
    // ===========================
    // Call this in -load mode every game loop cycle.
    // Returns 0 if no input happened at this specific time.
    virtual int popEventAtTime(long currentTime);

    // Resets the playback iterator to the beginning (for restarting)
    void resetPlayback() { playbackIndex = 0; }

    // ===========================
    //       File I/O
    // ===========================
    virtual bool saveFile(const std::string& filename) const;
    virtual bool loadFile(const std::string& filename);
};