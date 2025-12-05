#include "Riddle.h"

// Default: a trivial multiple-choice riddle with empty data.
Riddle::Riddle()
    : type(RiddleType::MultipleChoice),
    question(),
    options(),
    correctIndex(0),
    simonPattern(),
    simonVisibleDelayMs(400)
{
}

// Static factory for multiple-choice riddles
Riddle Riddle::makeMultipleChoice(
    const std::string& question,
    const std::array<std::string, 4>& options,
    int correctIndex)
{
    Riddle r;
    r.type = RiddleType::MultipleChoice;
    r.question = question;
    r.options = options;
    r.correctIndex = correctIndex;

    // Clear Simon data (not used for this type)
    r.simonPattern.clear();
    r.simonVisibleDelayMs = 400;

    return r;
}

// Static factory for Simon-Says riddles this is a bouns part
Riddle Riddle::makeSimonSays(
    const std::vector<int>& pattern,
    int delayMs)
{
    Riddle r;
    r.type = RiddleType::SimonSays;

    // Clear multiple-choice data (not used for this type)
    r.question.clear();
    r.options = {};
    r.correctIndex = 0;

    // Set Simon-Says data
    r.simonPattern = pattern;
    r.simonVisibleDelayMs = delayMs;

    return r;
}
