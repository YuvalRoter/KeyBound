#pragma once

#include <string>
#include <array>
#include <vector>

// Enum defines the specific behavior of the riddle
enum class RiddleType {
    MultipleChoice,
    SimonSays
};

class Riddle {
private:
    // ===========================
    //       Private Data
    // ===========================

    RiddleType type;

    // --- Multiple Choice Data ---
    std::string question;
    std::array<std::string, 4> options;
    int correctIndex;

    // --- Simon Says Data ---
    std::vector<int> simonPattern;      // Sequence of rectangle indices (0..3)
    int simonVisibleDelayMs;            // Duration of flash in ms

public:
    // ===========================
    //       Constructors
    // ===========================

    // Default: Creates an empty multiple-choice riddle
    Riddle();

    // ===========================
    //      Factory Methods
    // ===========================
    // Static functions to create specific types of riddles.
    // This is the "Factory Pattern".

    static Riddle makeMultipleChoice(
        const std::string& question,
        const std::array<std::string, 4>& options,
        int correctIndex
    );

    static Riddle makeSimonSays(
        const std::vector<int>& pattern,
        int delayMs = 400
    );

    // ===========================
    //        Accessors
    // ===========================
    // All marked 'const' because they do not modify the object.

    RiddleType getType() const { return type; }

    // -- For Multiple Choice --
    const std::string& getQuestion() const { return question; }
    const std::array<std::string, 4>& getOptions() const { return options; }
    int getCorrectIndex() const { return correctIndex; }

    // -- For Simon Says --
    const std::vector<int>& getSimonPattern() const { return simonPattern; }
    int getSimonDelayMs() const { return simonVisibleDelayMs; }
};