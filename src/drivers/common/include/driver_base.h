#pragma once

#include "input.h"

#include <string>
#include <map>
#include <vector>
#include <memory>

extern "C" typedef struct retro_core_t retro_core_t;

namespace drivers {

class buffered_audio;
class throttle;

/* base class for all drivers */
class driver_base {
    template<class T>
    friend driver_base *load_core(const std::string &path);

public:
    /* variable struct */
    struct variable_t {
        /* selected index */
        size_t curr_index;
        /* default index */
        size_t default_index;
        /* variable display text */
        std::string label;
        /* variable description */
        std::string info;
        /* visible */
        bool visible;
        /* options list, in pair (display, description) */
        std::vector<std::pair<std::string, std::string>> options;
    };

protected:
    driver_base();

public:
    /* construct with shared object file path for driver core */
    virtual ~driver_base();

    /* enter core main loop */
    virtual void run();

    /* load game from file with given path */
    void load_game(const std::string &path);

    /* unload game */
    void unload_game();

    /* do a hard rest of game */
    void reset();

    /* environment callback */
    bool env_callback(unsigned cmd, void *data);

    inline const std::string &get_system_dir() { return system_dir; }
    inline throttle *get_frame_throttle() { return frame_throttle.get(); }
    inline buffered_audio *get_audio() { return audio.get(); }

private:
    /* load core from path */
    bool load(const std::string &path);

    /* internal init/deinit */
    bool init_internal();
    void deinit_internal();

protected:
    /* virtual methods for cores init/deinit */
    virtual bool init() = 0;
    virtual void deinit() = 0;

    /* frame runner for the core */
    virtual bool run_frame() = 0;

    /* set when geometry or pixel_format is updated */
    virtual void geometry_updated() = 0;

public:
    /* virtual method for callback use */
    virtual void video_refresh(const void *data, unsigned width, unsigned height, size_t pitch) = 0;
    virtual void input_poll() = 0;
    virtual int16_t input_state(unsigned port, unsigned device, unsigned index, unsigned id);

protected:
    /* core struct, check libretro/include/core.h */
    retro_core_t *core = nullptr;

    /* various directories, as described in libretro env def */
    std::string system_dir;
    std::string libretro_dir;
    std::string save_dir;

    /* need_fullpath in retro_system_info, for game loading use */
    bool need_fullpath = false;

    /* retro_pixel_format, check libretro.h */
    unsigned pixel_format = 0;

    /* support no game boot */
    bool support_no_game = false;

    /* all variables copied from retro_system_av_info */
    unsigned base_width = 0;    /* Nominal video width of game. */
    unsigned base_height = 0;   /* Nominal video height of game. */
    unsigned max_width = 0;     /* Maximum possible width of game. */
    unsigned max_height = 0;    /* Maximum possible height of game. */

    float    aspect_ratio = 0.; /* Nominal aspect ratio of game. If
                                 * aspect_ratio is <= 0.0, an aspect ratio
                                 * of base_width / base_height is assumed.
                                 * A frontend could override this setting,
                                 * if desired. */

    /* input related variables */
    input pads;
    int16_t pad_states = 0;

    /* frame throttle */
    std::unique_ptr<throttle> frame_throttle;

    /* buffered_audio for audio input */
    std::unique_ptr<buffered_audio> audio;

private:
    /* game data */
    std::string game_data;

    /* variables */
    std::map<std::string, variable_t> variables;
    bool variables_updated = false;

    /* core is inited */
    bool inited = false;
};

template<class T>
inline driver_base *load_core(const std::string &path) {
    auto *c = new(std::nothrow) T;
    if (c == nullptr) return nullptr;
    if (!c->load(path)) {
        delete c;
        return nullptr;
    }
    return c;
}

}
