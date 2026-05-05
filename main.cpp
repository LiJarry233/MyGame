#include "Game.h"
#include <cstring>

int main(int argc, char** argv) {
    NetworkMode mode = NetworkMode::NONE;

    if (argc > 1) {
        if (strcmp(argv[1], "host") == 0) {
            mode = NetworkMode::HOST;
        } else if (strcmp(argv[1], "client") == 0) {
            mode = NetworkMode::CLIENT;
        }
    }

    Game game(mode);
    game.Run();
    return 0;
}