#include "GameManger.h"
#include "StepsSimple.h"
#include "StepsPlayer.h"
//#include "StepsRecorder.h"

int main(int argc, char* argv[]) {
    Steps* handler = nullptr;

    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "-load") {
            bool silent = (argc > 2 && std::string(argv[2]) == "-silent");
            handler = new StepsPlayer(silent);
        }/*
        else if (arg == "-save") {
            handler = new StepsRecorder();
        }
        */
    }

    // DEFAULT: If no args or invalid args, play normally (Simple Mode)
    if (handler == nullptr) {
        handler = new StepsSimple();
    }

    GameManger game(handler);
    game.run();

    delete handler;
    return 0;
}