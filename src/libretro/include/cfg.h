#pragma once

#include <vector>
#include <string>
#include <cstdint>

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
    void load();
    void save();

    inline const std::string &get_data_dir() const { return data_dir; }
    void set_data_dir(const std::string &dir);
    inline const std::string &get_store_dir() const { return store_dir; }
    void set_store_dir(const std::string &dir);
    inline const std::string &get_config_dir() const { return config_dir; }
    void get_core_dirs(std::vector<std::string> &dirs) const;
    void set_extra_core_dirs(const std::vector<std::string> &dirs);
    inline std::pair<uint32_t, uint32_t> get_resolution() { return std::make_pair(res_w, res_h); }
    inline bool get_mono_audio() const { return mono_audio; }
    inline void set_mono_audio(bool b) { mono_audio = b; }
    inline uint32_t get_sample_rate() const { return sample_rate; }
    inline void set_sample_rate(uint32_t s) { sample_rate = s; }
    inline uint32_t get_resampler_quality() const { return resampler_quality; }
    inline void set_resampler_quality(uint32_t r) { resampler_quality = r; }

    inline uint32_t get_scaling_mode() const { return scaling_mode; }
    inline void set_scaling_mode(uint32_t s) { scaling_mode = s; }
    inline uint32_t get_scale() const { return scale; }
    inline void set_scale(uint32_t s) { scale = s; }

    inline bool get_integer_scaling() const { return integer_scaling; }
    inline void set_integer_scaling(bool s) { integer_scaling = s; }
    inline bool get_linear() const { return linear; }
    inline void set_linear(bool l) { linear = l; }

    inline uint32_t get_save_check() const { return save_check; }
    inline void set_save_check(uint32_t c) { save_check = c; }

    inline int get_language() const { return language; }
    inline void set_language(int lang) { language = lang; }

protected:
    /* dir of static data */
    std::string data_dir;
    /* dir of dynamic data */
    std::string store_dir;
    /* dir of config files */
    std::string config_dir;
    /* extra dirs of libretro cores
     *  priority: core_dirs, data_dir, config_dir
     * */
    std::vector<std::string> core_dirs;

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

    /* === SDL1-only options === */
    /* scaling mode
     * 0  IPU scaling
     * 1  Screen center */
    uint32_t scaling_mode = 0;
    /* basic integer scaler */
    uint32_t scale = DEFAULT_SCALE;

    /* === SDL2-only options === */
    /* allow only integer scaling (will trim down to nearest integer scaling ratio),
     * this is helpful with liner rendering disabled */
    bool integer_scaling = false;
    /* use hardware linear rendering */
    bool linear = true;

    /* save check interval in seconds, set to 0 to disable it */
    uint32_t save_check = 0;

    /* ui langauge
     * check enum retro_language in libretro.h
     * */
    int language;
};

extern cfg g_cfg;
