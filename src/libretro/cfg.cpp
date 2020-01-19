#include "cfg.h"

#include "json.hpp"

#include <fstream>
#include <iostream>
#include <iomanip>

cfg g_cfg;

using json = nlohmann::json;

template<typename T>
T get_value(json &j, const std::string &key, T defval) {
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
        res_w = get_value<uint32_t>(j, "res_w", DEFAULT_WIDTH);
        res_h = get_value<uint32_t>(j, "res_h", DEFAULT_HEIGHT);
        mono_audio  = get_value<bool>(j, "mono_audio", false);
        sample_rate  = get_value<uint32_t>(j, "sample_rate", DEFAULT_SAMPLE_RATE);
        resampler_quality  = get_value<uint32_t>(j, "resampler_quality", DEFAULT_RESAMPLER_QUALITY);
        scale = get_value<uint32_t>(j, "scale", DEFAULT_SCALE);
        save_check = get_value<uint32_t>(j, "save_check", 0);
    }
}

void cfg::save() {
    std::ofstream ofs(filename, std::ios::binary | std::ios_base::out | std::ios_base::trunc);
    if (!ofs.good()) return;

    json j;
    j["res_w"] = res_w;
    j["res_h"] = res_h;
    j["mono_audio"] = mono_audio;
    j["sample_rate"] = sample_rate;
    j["resampler_quality"] = resampler_quality;
    j["scale"] = scale;
    j["save_check"] = save_check;

    try {
        ofs << std::setw(4) << j;
    } catch(...) {
        std::cerr << "failed to write config to " << filename << std::endl;
    }
    ofs.close();
}
