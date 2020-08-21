#include "cfg.h"

#include "util.h"

#include <json.hpp>
#include <spdlog/spdlog.h>

#include <cstdlib>

cfg g_cfg;

using json = nlohmann::json;
#ifdef _WIN32
#define realpath(r, f) _fullpath(f, r, PATH_MAX)
#endif

void cfg::set_data_dir(const std::string &dir) {
    char n[PATH_MAX];
    if (realpath(dir.c_str(), n) == nullptr)
        data_dir = dir;
    else
        data_dir = n;
}

void cfg::set_store_dir(const std::string &dir) {
    char n[PATH_MAX];
#ifdef _WIN32
    if (realpath(dir.c_str(), n) == nullptr)
        store_dir = dir;
    else
        store_dir = n;
    util::mkdir(store_dir, true);
#else
    util::mkdir(dir, true);
    if (realpath(dir.c_str(), n) == nullptr)
        store_dir = dir;
    else
        store_dir = n;
#endif
    config_dir = store_dir + PATH_SEPARATOR_CHAR + "cfg";
    util::mkdir(config_dir);
}

void cfg::set_extra_core_dirs(const std::vector<std::string> &dirs) {
    core_dirs.clear();
    for (const auto &d: dirs) {
        char n[PATH_MAX];
        if (realpath(d.c_str(), n) != nullptr)
            core_dirs.emplace_back(n);
    }
}

void cfg::get_core_dirs(std::vector<std::string> &dirs) const {
    dirs = core_dirs;
    auto ite = std::find(dirs.begin(), dirs.end(), data_dir);
    if (ite == core_dirs.end()) dirs.push_back(data_dir);
    ite = std::find(dirs.begin(), dirs.end(), config_dir);
    if (ite == core_dirs.end()) dirs.push_back(config_dir);
}

template<typename T>
inline T get_value(json &j, const std::string &key, T defval) {
    auto jsub = j[key];
    if (jsub.is_string() || jsub.is_boolean() || jsub.is_number()) {
        return jsub.get<T>();
    }
    return defval;
}

void cfg::load() {
    json j;
    auto filename = g_cfg.get_config_dir() + PATH_SEPARATOR_CHAR + "sdlretro.json";
    if (!util::file_exists(filename)) return;
    try {
        std::string content;
        if (!util::read_file(filename, content)) {
            throw std::bad_exception();
        }
        j = json::parse(content);
    } catch(...) {
        spdlog::error("failed to read config from {}", filename);
        return;
    }

    if (j.is_object()) {
#define JREAD(name, def) name = get_value<decltype(name)>(j, #name, (def))
        JREAD(res_w, DEFAULT_WIDTH);
        JREAD(res_h, DEFAULT_HEIGHT);
        JREAD(mono_audio, false);
        JREAD(sample_rate, DEFAULT_SAMPLE_RATE);
        JREAD(resampler_quality, DEFAULT_RESAMPLER_QUALITY);
        JREAD(scaling_mode, 0);
        JREAD(scale, DEFAULT_SCALE);
        JREAD(integer_scaling, false);
        JREAD(linear, true);
        JREAD(save_check, 0);
#undef JREAD
    }
}

void cfg::save() {
    json j;
#define JWRITE(name) j[#name] = name
    JWRITE(res_w);
    JWRITE(res_h);
    JWRITE(mono_audio);
    JWRITE(sample_rate);
    JWRITE(resampler_quality);
    JWRITE(scaling_mode);
    JWRITE(scale);
    JWRITE(integer_scaling);
    JWRITE(linear);
    JWRITE(save_check);
#undef JWRITE
    auto filename = g_cfg.get_config_dir() + PATH_SEPARATOR_CHAR + "sdlretro.json";
    try {
        auto content = j.dump(4);
        if (!util::write_file(filename, content))
            throw std::bad_exception();
    } catch(...) {
        spdlog::error("failed to write config to {}", filename);
    }
}
