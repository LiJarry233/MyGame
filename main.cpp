#include "Game.h"
#include <cstring>

int main(int argc, char** argv) {
    NetworkMode mode = NetworkMode::NONE;
    bool benchmark = false;
    const char* benchTag = "result";

    if (argc > 1) {
        if (strcmp(argv[1], "host") == 0) {
            mode = NetworkMode::HOST;
        } else if (strcmp(argv[1], "client") == 0) {
            mode = NetworkMode::CLIENT;
        } else if (strcmp(argv[1], "benchmark") == 0) {
            benchmark = true;
            if (argc > 2) benchTag = argv[2];
        }
    }

    Game game(mode, benchmark);
    if (benchmark) {
        game.RunBenchmark(benchTag);
    } else {
        game.Run();
    }
    return 0;
}