#include "sdl1_impl.h"

#include <cstdio>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: sdlretro <core file> <rom file>");
        return 1;
    }
    setvbuf(stdout, 0, _IONBF, 0);
    setvbuf(stderr, 0, _IONBF, 0);
    auto *impl = drivers::load_core<drivers::sdl1_impl>(argv[1]);
    if (!impl) {
        fprintf(stderr, "unable to load core!\n");
        return 1;
    }
    impl->load_game(argv[2]);
    impl->run();
    impl->unload_game();
    return 0;
}
