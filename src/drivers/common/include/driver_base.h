#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <functional>

extern "C" typedef struct retro_core_t retro_core_t;

namespace libretro {
class retro_variables;
}

namespace drivers {

class video_base;
class buffered_audio;
class input_base;
class throttle;

/* base class for all drivers */
class driver_base {
protected:
    driver_base();

public:
    /* construct with shared object file path for driver core */
    virtual ~driver_base();

    /* enter core main loop */
    void run(std::function<void()> in_game_menu_cb);

    /* shutdown the driver */
    inline void shutdown() { shutdown_driver = true; }

    /* load game from file with given path */
    bool load_game(const std::string &path);

    /* load game from memory, use temp file if mem load is not supported */
    bool load_game_from_mem(const std::string &path, const std::string &ext, const std::vector<uint8_t> &data);

    /* unload game */
    void unload_game();

    /* do a hard rest of game */
    void reset();

    /* environment callback */
    bool env_callback(unsigned cmd, void *data);
    void save_variables_to_cfg();

    inline const std::string &get_system_dir() const { return system_dir; }
    inline throttle *get_frame_throttle() { return frame_throttle.get(); }
    inline video_base *get_video() { return video.get(); }
    inline buffered_audio *get_audio() { return audio.get(); }
    inline input_base *get_input() { return input.get(); }
    inline libretro::retro_variables *get_variables() { return variables.get(); }

    /* load core from path */
    bool load_core(const std::string &path);

    /* process events, for menu use, return true for QUIT event */
    virtual bool process_events() { return false; }

private:
    /* internal init/deinit */
    bool init_internal();
    void deinit_internal();

    /* check sram/rtc and save to file if changed */
    void check_save_ram();

    /* post processing for game load */
    void post_load();

protected:
    /* virtual methods for cores init/deinit */
    virtual bool init() = 0;
    virtual void deinit() = 0;
    virtual void unload() = 0;

    /* frame runner for the core */
    virtual bool run_frame(std::function<void()> &in_game_menu_cb, bool check);

protected:
    /* core struct, check libretro/include/core.h */
    retro_core_t *core = nullptr;

    /* various directories, as described in libretro env def */
    std::string system_dir; /* `static_dir`/system */
    std::string save_dir;   /* `config_dir`/saves */

    /* retro_pixel_format, check libretro.h */
    unsigned pixel_format = 2;

    /* support no game boot */
    bool support_no_game = false;

    /* support achivements, but it might not be implemented by SDLRetro */
    bool support_achivements = false;

    /* all variables copied from retro_system_info */
    std::string library_name;
    std::string library_version;
    bool need_fullpath = false;

    /* save_dir + '/' + lower-cased library_name */
    std::string core_save_dir;

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
    std::shared_ptr<throttle> frame_throttle;

    /* video_base for video output */
    std::shared_ptr<video_base> video;

    /* buffered_audio for audio input */
    std::shared_ptr<buffered_audio> audio;

    /* input_base for contoller input */
    std::shared_ptr<input_base> input;

    /* varaibles */
    std::unique_ptr<libretro::retro_variables> variables;

    /* menu button was pressed */
    bool menu_button_pressed = false;

private:
    /* core cfg file path */
    std::string core_cfg_path;

    /* game file path and base name(remove dir and ext part from game path) */
    std::string game_path;
    std::string game_base_name;

    /* game save/rtc path */
    std::string game_save_path;
    std::string game_rtc_path;

    /* game data */
    std::string game_data;

    /* temp file for unzip, would be removed after gameplay */
    std::string temp_file;

    /* save&rtc data */
    std::vector<uint8_t> save_data;
    std::vector<uint8_t> rtc_data;

    /* frame countdown for save check */
    uint32_t save_check_countdown = 0;

    /* core is inited */
    bool inited = false;

    /* shutdown is requested */
    bool shutdown_driver = false;
};

template<class T>
inline std::shared_ptr<driver_base> create_driver() {
    driver_base *c = new(std::nothrow) T;
    if (c == nullptr) return std::shared_ptr<driver_base>();
    return std::shared_ptr<driver_base>(c);
}

}
