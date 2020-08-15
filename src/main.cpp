#include "miniz.h"

#if SDLRETRO_FRONTEND == 1
#include <sdl1_impl.h>
#endif
#if SDLRETRO_FRONTEND == 2
#include <sdl2_impl.h>
#endif
#include <core_manager.h>
#include <ui_menu.h>
#include <cfg.h>

#include <cstdio>
#include <cstring>

#ifdef _WIN32
#define PATH_SEPARATOR_CHAR "\\"
#else
#define PATH_SEPARATOR_CHAR "/"
#endif

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: sdlretro <rom file>");
        return 1;
    }
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);

#if SDLRETRO_FRONTEND == 1
    auto impl = drivers::create_driver<drivers::sdl1_impl>();
#endif
#if SDLRETRO_FRONTEND == 2
    auto impl = drivers::create_driver<drivers::sdl2_impl>();
#endif
    if (!impl) {
        fprintf(stderr, "Unable to create driver!\n");
        return 1;
    }

#ifdef GCW_ZERO
    const std::string static_root(".");
    const std::string config_root("/usr/local/home/.sdlretro");
    libretro::core_manager coreman({static_root, config_root});
#else
    const std::string static_root(".");
    const std::string config_root(".");
    libretro::core_manager coreman({static_root});
#endif

    g_cfg.set_filename(config_root + PATH_SEPARATOR_CHAR + "sdlretro.json");
    g_cfg.load();

    impl->set_dirs(static_root, config_root);
    gui::ui_menu menu(impl);

    const char *ptr = strrchr(argv[1], '.');
    if (ptr == nullptr) {
        fprintf(stderr, "Cannot find core for file w/o extension!\n");
        return 1;
    }

    std::vector<const libretro::core_info*> core_list;
    std::vector<uint8_t> unzipped_data;
    std::string ext;
    if (strcasecmp(ptr, ".zip") == 0) {
        do {
            mz_zip_archive arc = {};
            if (!mz_zip_reader_init_file(&arc, argv[1], 0)) break;
            auto num_files = mz_zip_reader_get_num_files(&arc);
            if (num_files == 0) {
                fprintf(stderr, "Empty zip file!\n");
                mz_zip_reader_end(&arc);
                return 1;
            }
            if (num_files > 2)
                break;
            for (uint32_t i = 0; i < num_files; ++i) {
                mz_zip_archive_file_stat file_stat;
                mz_zip_reader_file_stat(&arc, 0, &file_stat);
                const char *ext_ptr = strrchr(file_stat.m_filename, '.');
                if (ext_ptr == nullptr || strcasecmp(ext_ptr, ".txt")==0) continue;
                ext = ext_ptr + 1;
                core_list = coreman.match_cores_by_extension(ext);
                if (!core_list.empty()) {
                    unzipped_data.resize(file_stat.m_uncomp_size);
                    mz_zip_reader_extract_to_mem(&arc, 0, &unzipped_data[0], file_stat.m_uncomp_size, 0);
                    break;
                }
            }
            mz_zip_reader_end(&arc);
        } while (false);
    }
    if (core_list.empty()) {
        ext = ptr + 1;
        core_list = coreman.match_cores_by_extension(ext);
        if (core_list.empty()) {
            fprintf(stderr, "Cannot find core for file extension %s!\n", ext.c_str());
            return 1;
        }
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

    if (unzipped_data.empty()) {
        impl->load_game(argv[1]);
    } else {
        impl->load_game_from_mem(argv[1], ext, unzipped_data);
    }
    impl->run(std::bind(&gui::ui_menu::in_game_menu, &menu));
    impl->unload_game();

    return 0;
}
