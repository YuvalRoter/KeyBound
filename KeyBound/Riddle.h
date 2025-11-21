#pragma once

#include <string>
#include <array>
#include <vector>

// What kind of riddle this is.
enum class RiddleType {
    MultipleChoice,
    SimonSays
};




class Riddle {
public:
    // Default constructor: makes a trivial multiple-choice riddle.
    Riddle();

    // Factory: create a multiple-choice riddle
    static Riddle makeMultipleChoice(
        const std::string& question,
        const std::array<std::string, 4>& options,
        int correctIndex  // 0..3 (index in options array)
    );

    // Factory: create a Simon-Says riddle
    // pattern: sequence of rectangle indices (0..3)
    static Riddle makeSimonSays(
        const std::vector<int>& pattern,
        int delayMs = 400  // how long each flash is visible
    );

    // ---- Getters ----
    RiddleType getType() const { return type; }

    // For MultipleChoice
    const std::string& getQuestion() const { return question; }
    const std::array<std::string, 4>& getOptions() const { return options; }
    int getCorrectIndex() const { return correctIndex; }

    // For SimonSays
    const std::vector<int>& getSimonPattern() const { return simonPattern; }
    int getSimonDelayMs() const { return simonVisibleDelayMs; }

private:
    // Which variant is active
    RiddleType type;

    // --- Multiple choice data ---
    std::string question;                     // question text
    std::array<std::string, 4> options;       // answers "1".."4"
    int correctIndex;                         // 0..3

    // --- Simon-Says data ---
    std::vector<int> simonPattern;            // sequence of 0..3 (rect indices)
    int simonVisibleDelayMs;                  // per-flash delay in ms
};
