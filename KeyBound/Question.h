#pragma once
#include <string>

struct Question {
    std::string text;
    std::string options[4];
    int correctIndex;
    std::string explanation;
};
