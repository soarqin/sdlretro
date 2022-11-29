#include "miniz.h"

#include <logger.h>

#if SDLRETRO_FRONTEND == 1
#include <sdl1_impl.h>
#endif
#if SDLRETRO_FRONTEND == 2
#include <sdl2_impl.h>
#endif
#include <i18n.h>
#include <core_manager.h>
#include <helper.h>
#include <ui_host.h>
#include <cfg.h>

#include <cstdio>
#include <cstring>

#include <getopt.h>

#define DEFAULT_DATA_DIR "."
#ifdef GCW_ZERO
#define DEFAULT_STORE_DIR "/usr/local/home/.sdlretro"
#else
#define DEFAULT_STORE_DIR "."
#endif

int program(int argc, char *argv[]) {
    const char *core_filename = nullptr;
    const char *config_filename = nullptr;
    static struct option long_options[] = {
        {"libretro",     required_argument, 0,  'L' },
        {"config",     required_argument, 0,  'c' },
        {nullptr }
    };
    opterr = 0;
    while (true) {
        int option_index = 0;
        int c = getopt_long(argc, argv, "L:", long_options, &option_index);
        if (c == -1) break;
        switch (c) {
        case 'L':
            core_filename = optarg;
            break;
        case 'c':
            config_filename = optarg;
            break;
        case '?':
            if (optopt)
                LOG(ERROR, "Bad option '-{}'", optopt);
            else
                LOG(ERROR, "Bad option '{}'", argv[optind - 1]);
            return 1;
        default:
            break;
        }
    }
    if (optind >= argc) {
        LOG(ERROR, "ROM filename missing.");
        return 1;
    }
    const char *rom_filename = argv[optind];

    if (config_filename) {
        g_cfg.load(config_filename);
        if (g_cfg.get_data_dir().empty())
            g_cfg.set_data_dir(DEFAULT_DATA_DIR);
        if (g_cfg.get_store_dir().empty())
            g_cfg.set_store_dir(DEFAULT_STORE_DIR);
    } else {
        g_cfg.set_data_dir(DEFAULT_DATA_DIR);
        g_cfg.set_store_dir(DEFAULT_STORE_DIR);
        g_cfg.load("");
    }

    libretro::i18n_obj.set_language(g_cfg.get_language());

    libretro::core_manager coreman;

#if SDLRETRO_FRONTEND == 1
    auto impl = drivers::create_driver<drivers::sdl1_impl>();
#endif
#if SDLRETRO_FRONTEND == 2
    auto impl = drivers::create_driver<drivers::sdl2_impl>();
#endif
    if (!impl) {
        LOG(ERROR, "Unable to create driver!");
        return 1;
    }

    gui::ui_host ui(impl);

    std::vector<uint8_t> unzipped_data;
    std::string rom_ext;

    std::string core_filepath;
    if (core_filename) {
        core_filepath = core_filename;
        if (!helper::file_exists(core_filepath)) {
            std::vector<std::string> dirs;
            g_cfg.get_core_dirs(dirs);
            bool found = false;
            for (auto &d: dirs) {
                core_filepath = d + PATH_SEPARATOR_CHAR + core_filename;
                if (helper::file_exists(core_filepath)) {
                    found = true;
                    break;
                }
                core_filepath = d + PATH_SEPARATOR_CHAR + core_filename + "." DYNLIB_EXTENSION;
                if (helper::file_exists(core_filepath)) {
                    found = true;
                    break;
                }
                core_filepath = d + PATH_SEPARATOR_CHAR + core_filename + "_libretro." DYNLIB_EXTENSION;
                if (helper::file_exists(core_filepath)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                LOG(ERROR, "Unable to load core '{}'!", core_filename);
                return 1;
            }
        }
    } else {
        const char *ptr = strrchr(rom_filename, '.');
        if (ptr == nullptr) {
            LOG(ERROR, "Cannot find core for file w/o extension!");
            return 1;
        }

        std::vector<const libretro::core_info *> core_list;
        if (strcasecmp(ptr, ".zip") == 0) {
            do {
                mz_zip_archive arc = {};
                if (!mz_zip_reader_init_file(&arc, rom_filename, 0)) break;
                auto num_files = mz_zip_reader_get_num_files(&arc);
                if (num_files == 0) {
                    LOG(ERROR, "Empty zip file!");
                    mz_zip_reader_end(&arc);
                    return 1;
                }
                if (num_files > 2)
                    break;
                for (uint32_t i = 0; i < num_files; ++i) {
                    mz_zip_archive_file_stat file_stat;
                    mz_zip_reader_file_stat(&arc, 0, &file_stat);
                    const char *ext_ptr = strrchr(file_stat.m_filename, '.');
                    if (ext_ptr == nullptr || strcasecmp(ext_ptr, ".txt") == 0) continue;
                    rom_ext = ext_ptr + 1;
                    core_list = coreman.match_cores_by_extension(rom_ext);
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
            rom_ext = ptr + 1;
            core_list = coreman.match_cores_by_extension(rom_ext);
            if (core_list.empty()) {
                LOG(ERROR, "Cannot find core for file extension {}!", rom_ext.c_str());
                return 1;
            }
        }
        int index = 0;
        if (core_list.size() > 1) {
            index = ui.select_core_menu(core_list);
            if (index < 0 || index >= core_list.size()) {
                return 1;
            }
        }
        core_filepath = core_list[index]->filepath;
    }
    if (!impl->load_core(core_filepath)) {
        LOG(ERROR, "Unable to load core from '{}'!", core_filepath);
        return 1;
    }
    if (unzipped_data.empty()) {
        impl->load_game(rom_filename);
    } else {
        impl->load_game_from_mem(rom_filename, rom_ext, unzipped_data);
    }
    impl->run([&ui] { ui.in_game_menu(); });
    impl->unload_game();

    return 0;
}

int main(int argc, char *argv[]) {
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);

#ifdef NDEBUG
    util::logInit(stdout, util::LogLevel::INFO);
#else
    util::logInit(stdout, util::LogLevel::TRACE);
#endif
    int res = program(argc, argv);
    util::logUninit();
    return res;
}

#ifdef _MSC_VER

#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow) {
    return main(__argc, __argv);
}

#endif
