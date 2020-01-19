#pragma once

#include <cstdint>
#include <string>

enum :uint32_t {
#ifdef GCW_ZERO
    DEFAULT_WIDTH = 320,
    DEFAULT_HEIGHT = 240,
    DEFAULT_SCALE = 1,
#else
    DEFAULT_WIDTH = 640,
    DEFAULT_HEIGHT = 480,
    DEFAULT_SCALE = 2,
#endif
    DEFAULT_SAMPLE_RATE = 0,
    DEFAULT_RESAMPLER_QUALITY = 0,
};

class cfg {
public:
    virtual ~cfg() = default;
    inline void set_filename(const std::string &name) { filename = name; }
    void load();
    void save();

    inline std::pair<uint32_t, uint32_t> get_resolution() { return std::make_pair(res_w, res_h); }
    inline bool get_mono_audio() { return mono_audio; }
    inline void set_mono_audio(bool b) { mono_audio = b; }
    inline uint32_t get_sample_rate() { return sample_rate; }
    inline void set_sample_rate(uint32_t s) { sample_rate = s; }
    inline uint32_t get_resampler_quality() { return resampler_quality; }
    inline void set_resampler_quality(uint32_t r) { resampler_quality = r; }
    inline uint32_t get_scale() { return scale; }
    inline void set_scale(uint32_t s) { scale = s; }
    inline uint32_t get_save_check() { return save_check; }
    inline void set_save_check(uint32_t c) { save_check = c; }

protected:
    std::string filename;

    uint32_t res_w = DEFAULT_WIDTH, res_h = DEFAULT_HEIGHT;
    /* set to true if use mono audio */
    bool mono_audio = false;
    /* set to 0 to use source sample rate with integer multiplier
     *   to reduce it to no more than 48kHz.
     * otherwise libsamplerate will be used to do resampling*/
    uint32_t sample_rate = DEFAULT_SAMPLE_RATE;
    /* libsamplerate resampler quality:
     * 4  SRC_SINC_BEST_QUALITY
     * 3  SRC_SINC_MEDIUM_QUALITY
     * 2  SRC_SINC_FASTEST
     * 1  SRC_ZERO_ORDER_HOLD
     * 0  SRC_LINEAR */
    uint32_t resampler_quality = DEFAULT_RESAMPLER_QUALITY;
    /* basic integer scaler */
    uint32_t scale = DEFAULT_SCALE;
    /* save check interval in seconds, set to 0 to disable it */
    uint32_t save_check = 0;
};

extern cfg g_cfg;
