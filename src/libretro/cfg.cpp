#include "cfg.h"

#include <util.h>

#include "json.hpp"

#include <iostream>

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
    json j;
    try {
        std::string content;
        if (!util_read_file(filename, content)) {
            throw std::bad_exception();
        }
        j = json::parse(content);
    } catch(...) {
        std::cerr << "failed to read config from " << filename << std::endl;
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
    JWRITE(save_check);
#undef JWRITE
    try {
        auto content = j.dump(4);
        if (!util_write_file(filename, content))
            throw std::bad_exception();
    } catch(...) {
        std::cerr << "failed to write config to " << filename << std::endl;
    }
}
