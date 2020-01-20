#include "cfg.h"

#include "json.hpp"

#include <fstream>
#include <iostream>
#include <iomanip>

cfg g_cfg;

using json = nlohmann::json;

template<typename T>
inline T get_value(json &j, const std::string &key, T defval) {
    auto jsub = j[key];
    if (jsub.is_string() || jsub.is_boolean() || jsub.is_number()) {
        return jsub.get<T>();
    }
    return defval;
}

void cfg::load() {
    std::ifstream ifs(filename, std::ios_base::binary | std::ios_base::in);
    if (!ifs.good()) return;

    json j;
    try {
        ifs >> j;
    } catch(...) {
        std::cerr << "failed to read config from " << filename << std::endl;
        ifs.close();
        return;
    }
    ifs.close();

    if (j.is_object()) {
#define JREAD(name, def) name = get_value<decltype(name)>(j, #name, (def))
        JREAD(res_w, DEFAULT_WIDTH);
        JREAD(res_h, DEFAULT_HEIGHT);
        JREAD(mono_audio, false);
        JREAD(sample_rate, DEFAULT_SAMPLE_RATE);
        JREAD(resampler_quality, DEFAULT_RESAMPLER_QUALITY);
        JREAD(scaling_mode, 0);
        JREAD(scale, DEFAULT_SCALE);
        JREAD(save_check, 0);
#undef JREAD
    }
}

void cfg::save() {
    std::ofstream ofs(filename, std::ios::binary | std::ios_base::out | std::ios_base::trunc);
    if (!ofs.good()) return;

    json j;
#define JWRITE(name) j[#name] = name
    JWRITE(res_w);
    JWRITE(res_h);
    JWRITE(mono_audio);
    JWRITE(sample_rate);
    JWRITE(resampler_quality);
    JWRITE(scaling_mode);
    JWRITE(scale);
    JWRITE(save_check);
#undef JWRITE
    try {
        ofs << std::setw(4) << j;
    } catch(...) {
        std::cerr << "failed to write config to " << filename << std::endl;
    }
    ofs.close();
}
