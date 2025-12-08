#pragma once
#include <string>
#include <array>

// Forward declaration
class GameManger;

class Question {
private:
    // ===========================
    //       Private Data
    // ===========================
    std::string text;
    std::array<std::string, 4> options;
    int correctIndex = 0;
    std::string explanation;

public:
    // ===========================
    //       Constructors
    // ===========================
    Question() = default;

    // ===========================
    //       Friend Classes
    // ===========================
    // Allows GameManger to load data directly into these fields
    // without needing complex setters.
    friend class GameManger;
};