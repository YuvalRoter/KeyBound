#include <string>
#include <array>

class Riddle {
public:

    std::string question;
    std::array<std::string, 4> options; // answers 1-4
    int correctIndex;                   // 0..3
    int RiddleType;
  
    std::string riddleMapFile;  //

    Riddle(const std::string& q,
        const std::array<std::string, 4>& opts,
        int correct,
        const std::string& mapFile = {})
        : question(q),
        options(opts),
        correctIndex(correct),
        riddleMapFile(mapFile)
    {
    }
};
