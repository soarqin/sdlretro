#include "cfg.h"

#include "util.h"

#include <json.hpp>
#include <spdlog/spdlog.h>

#include <cstdlib>

cfg g_cfg;

using json = nlohmann::json;
#ifdef _WIN32
#include <windows.h>
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#define realpath(r, f) _fullpath(f, r, PATH_MAX)
#endif

void cfg::set_data_dir(const std::string &dir) {
    char n[PATH_MAX];
    data_dir_orig = dir;
    if (realpath(dir.c_str(), n) == nullptr)
        data_dir = dir;
    else
        data_dir = n;
}

void cfg::set_store_dir(const std::string &dir) {
    char n[PATH_MAX];
    store_dir_orig = dir;
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
    if (ite == dirs.end()) dirs.push_back(data_dir);
    ite = std::find(dirs.begin(), dirs.end(), store_dir);
    if (ite == dirs.end()) dirs.push_back(store_dir);
    for (auto &d: dirs) {
        d += PATH_SEPARATOR_CHAR "cores";
    }
}

template<typename T>
inline bool get_value(json &j, const std::string &key, T &val, const T &defval) {
    auto jsub = j[key];
    if (jsub.is_string() || jsub.is_boolean() || jsub.is_number()) {
        val = jsub.get<T>();
        return true;
    }
    val = defval;
    return false;
}

template<typename T>
inline bool get_value(json &j, const std::string &key, T &val) {
    auto jsub = j[key];
    if (jsub.is_string() || jsub.is_boolean() || jsub.is_number()) {
        val = jsub.get<T>();
        return true;
    }
    return false;
}

void cfg::load(const std::string &cfgfile) {
    json j;
    config_filename = cfgfile.empty() ? config_dir + PATH_SEPARATOR_CHAR + "sdlretro.json" : cfgfile;
    if (!util::file_exists(config_filename)) return;
    try {
        std::string content;
        if (!util::read_file(config_filename, content)) {
            throw std::bad_exception();
        }
        j = json::parse(content);
    } catch(...) {
        spdlog::error("failed to read config from {}", config_filename);
        return;
    }

    if (j.is_object()) {
#define JREAD(name, def) get_value<decltype(name)>(j, #name, name, (def))
#define JREAD2(name) get_value<decltype(name)>(j, #name, name)
        std::string datadir;
        std::string storedir;
        if (JREAD2(datadir)) {
            set_data_dir(datadir);
        }
        if (JREAD2(storedir)) {
            set_store_dir(storedir);
        }
        JREAD(res_w, DEFAULT_WIDTH);
        JREAD(res_h, DEFAULT_HEIGHT);
        JREAD(fullscreen, false);
        JREAD(mono_audio, false);
        JREAD(sample_rate, DEFAULT_SAMPLE_RATE);
        JREAD(resampler_quality, DEFAULT_RESAMPLER_QUALITY);
        JREAD(scaling_mode, 0);
        JREAD(scale, DEFAULT_SCALE);
        JREAD(integer_scaling, false);
        JREAD(linear, true);
        JREAD(save_check, 0);
        JREAD(language, 0);
#undef JREAD
    }
}

void cfg::save() {
    json j;
#define JWRITE(name) j[#name] = name
    j["datadir"] = data_dir_orig;
    j["storedir"] = store_dir_orig;
    JWRITE(res_w);
    JWRITE(res_h);
    JWRITE(fullscreen);
    JWRITE(mono_audio);
    JWRITE(sample_rate);
    JWRITE(resampler_quality);
    JWRITE(scaling_mode);
    JWRITE(scale);
    JWRITE(integer_scaling);
    JWRITE(linear);
    JWRITE(save_check);
    JWRITE(language);
#undef JWRITE
    try {
        auto content = j.dump(4);
        if (!util::write_file(config_filename, content))
            throw std::bad_exception();
    } catch(...) {
        spdlog::error("failed to write config to {}", config_filename);
    }
}
