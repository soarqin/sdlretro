#include <sdl1_impl.h>
#include <core_manager.h>

#include <cstdio>
#include <cstring>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: sdlretro <rom file> [<core file>]");
        return 1;
    }
    setvbuf(stdout, 0, _IONBF, 0);
    setvbuf(stderr, 0, _IONBF, 0);

    std::string core_file;
    if (argc < 3) {
        drivers::core_manager coreman(".");
        const char *ptr = strrchr(argv[1], '.');
        if (ptr == nullptr) {
            fprintf(stderr, "cannot find core for file w/o extension!\n");
            return 1;
        }
        auto core_list = coreman.match_cores_by_extension(ptr + 1);
        if (core_list.empty()) {
            fprintf(stderr, "cannot find core for file extension %s\n", ptr + 1);
            return 1;
        }
        core_file = core_list[0].filepath;
    } else {
        core_file = argv[2];
    }
    auto impl = drivers::load_core<drivers::sdl1_impl>(core_file);
    if (!impl) {
        fprintf(stderr, "unable to load core!\n");
        return 1;
    }
    impl->load_game(argv[1]);
    impl->run();
    impl->unload_game();

    return 0;
}
