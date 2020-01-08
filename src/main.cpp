#include <sdl1_impl.h>
#include <core_manager.h>
#include <ui_menu.h>
#include <cfg.h>

#include <cstdio>
#include <cstring>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: sdlretro <rom file> [<core file>]");
        return 1;
    }
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);

#ifdef GCW_ZERO
    const std::string static_root(".");
    const std::string config_root("/usr/local/home/.sdlretro");
#else
    const std::string static_root(".");
    const std::string config_root(".");
#endif

    g_cfg.set_filename(config_root + "/sdlretro.json");
    g_cfg.load();

    libretro::core_manager coreman(static_root, config_root);
    auto impl = drivers::create_driver<drivers::sdl1_impl>();
    if (!impl) {
        fprintf(stderr, "Unable to create driver!\n");
        return 1;
    }
    impl->set_dirs(static_root, config_root);
    gui::ui_menu menu(impl);
    if (argc < 3) {
        const char *ptr = strrchr(argv[1], '.');
        if (ptr == nullptr) {
            fprintf(stderr, "Cannot find core for file w/o extension!\n");
            return 1;
        }
        auto core_list = coreman.match_cores_by_extension(ptr + 1);
        if (core_list.empty()) {
            fprintf(stderr, "Cannot find core for file extension %s!\n", ptr + 1);
            return 1;
        }
        int index = 0;
        if (core_list.size() > 1) {
            index = menu.select_core_menu(core_list);
            if (index < 0 || index >= core_list.size()) {
                return 1;
            }
        }
        if (!impl->load_core(core_list[index]->filepath)) {
            fprintf(stderr, "Unable to load core from `%s`!\n", core_list[index]->filepath.c_str());
            return 1;
        }
    } else {
        if (!impl->load_core(argv[2])) {
            fprintf(stderr, "Unable to load core from `%s`!\n", argv[2]);
            return 1;
        }
    }

    impl->load_game(argv[1]);
    impl->run(std::bind(&gui::ui_menu::in_game_menu, &menu));
    impl->unload_game();

    return 0;
}
