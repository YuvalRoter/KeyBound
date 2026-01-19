#include "Steps.h"

bool Steps::saveFile(const std::string& filename) const {
    std::ofstream outfile(filename);
    if (!outfile) return false;

    outfile << randomSeed << "\n";
    outfile << mapFiles.size();
    for (const auto& f : mapFiles) outfile << " " << f;
    outfile << "\n";

    for (const auto& s : recordedSteps) {
        outfile << s.time << " " << s.input << "\n";
    }
    return true;
}

bool Steps::loadFile(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile) return false;

    recordedSteps.clear();
    mapFiles.clear();
    playbackIndex = 0;

    if (!(infile >> randomSeed)) return false;

    size_t count;
    if (!(infile >> count)) return false;
    for (size_t i = 0; i < count; ++i) {
        std::string s;
        infile >> s;
        mapFiles.push_back(s);
    }

    long t;
    int d;
    while (infile >> t >> d) {
        recordedSteps.push_back({ t, d });
    }
    return true;
}

int Steps::popEventAtTime(long currentTime) {
    if (playbackIndex >= recordedSteps.size()) return 0;

    if (recordedSteps[playbackIndex].time == currentTime) {
        return recordedSteps[playbackIndex++].input;
    }
    return 0;
}