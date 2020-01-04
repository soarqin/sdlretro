#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>

extern "C" typedef struct retro_core_t retro_core_t;

namespace drivers {

class video_base;
class buffered_audio;
class input_base;
class throttle;

/* base class for all drivers */
class driver_base {
    friend class core_manager;

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

    /* set static root folder and config root folder */
    void set_dirs(const std::string &static_root, const std::string &config_root);

    /* enter core main loop */
    virtual void run();

    /* load game from file with given path */
    bool load_game(const std::string &path);

    /* unload game */
    void unload_game();

    /* do a hard rest of game */
    void reset();

    /* environment callback */
    bool env_callback(unsigned cmd, void *data);

    inline const std::string &get_system_dir() { return system_dir; }
    inline throttle *get_frame_throttle() { return frame_throttle.get(); }
    inline video_base *get_video() { return video.get(); }
    inline buffered_audio *get_audio() { return audio.get(); }
    inline input_base *get_input() { return input.get(); }

private:
    /* load core from path */
    bool load(const std::string &path);

    /* internal init/deinit */
    bool init_internal();
    void deinit_internal();

    /* check sram/rtc and save to file if changed */
    void check_save_ram();

protected:
    /* virtual methods for cores init/deinit */
    virtual bool init() = 0;
    virtual void deinit() = 0;
    virtual void unload() = 0;

    /* frame runner for the core */
    virtual bool run_frame() = 0;

protected:
    /* core struct, check libretro/include/core.h */
    retro_core_t *core = nullptr;

    /* various directories, as described in libretro env def */
    std::string system_dir;
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
    double   fps = 0.;

    /* frame throttle */
    std::unique_ptr<throttle> frame_throttle;

    /* video_base for video output */
    std::unique_ptr<video_base> video;

    /* buffered_audio for audio input */
    std::unique_ptr<buffered_audio> audio;

    /* input_base for contoller input */
    std::unique_ptr<input_base> input;

private:
    /* game file path and base name(remove dir and ext part from game path) */
    std::string game_path;
    std::string game_base_name;

    /* game save/rtc path */
    std::string game_save_path;
    std::string game_rtc_path;

    /* game data */
    std::string game_data;

    /* save&rtc data */
    std::vector<uint8_t> save_data;
    std::vector<uint8_t> rtc_data;

    /* frame countdown for save check */
    uint32_t save_check_countdown = 0;

    /* variables */
    std::map<std::string, variable_t> variables;
    bool variables_updated = false;

    /* core is inited */
    bool inited = false;
};

}
