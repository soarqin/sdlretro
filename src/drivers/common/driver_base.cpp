#include "driver_base.h"

#include "cfg.h"
#include "logger.h"

#include "video_base.h"
#include "audio_base.h"
#include "input_base.h"
#include "throttle.h"

#include <variables.h>
#include <i18n.h>
#include <core.h>
#include <helper.h>
#include <perf.h>

#include <memory>
#include <cstring>
#include <cstdarg>
#include <cmath>

namespace drivers {

inline void lowered_string(std::string &s) {
    for (char &c: s) {
        if (c <= ' ' || c == '\\' || c == '/' || c == ':' || c == '*' || c == '"' || c == '<' || c == '>' || c == '|')
            c = '_';
        else
            c = (char)std::tolower(c);
    }
}

inline std::string get_base_name(const std::string &path) {
    std::string basename = path;
    auto pos = basename.find_last_of("/\\");
    if (pos != std::string::npos) {
        basename = basename.substr(pos + 1);
    }
    pos = basename.find_last_of('.');
    if (pos != std::string::npos)
        basename.erase(pos);
    return basename;
}

driver_base *current_driver = nullptr;

driver_base::driver_base() {
    frame_throttle = std::make_shared<throttle>();
    variables = std::make_unique<libretro::retro_variables>();

    system_dir = g_cfg.get_store_dir() + PATH_SEPARATOR_CHAR "system";
    helper::mkdir(system_dir);
    save_dir = g_cfg.get_store_dir() + PATH_SEPARATOR_CHAR "saves";
    helper::mkdir(save_dir);
}

driver_base::~driver_base() {
    deinit_internal();

    if (core) {
        core_unload(core);
        core = nullptr;
    }
    if (keyboard_callback) {
        delete keyboard_callback;
        keyboard_callback = nullptr;
    }
    if (disk_control_callback) {
        delete disk_control_callback;
        disk_control_callback = nullptr;
    }

    current_driver = nullptr;
}

void driver_base::run(const std::function<void()> &in_game_menu_cb) {
    while (!shutdown_driver && !process_events()) {
        if (menu_button_pressed) {
            audio->pause(true);
            in_game_menu_cb();
            audio->pause(false);
            frame_throttle->reset(fps);
            menu_button_pressed = false;
        }

        core->retro_run();
        if (video->frame_drawn()) {
            int64_t usecs = frame_throttle->check_wait();
            if (usecs > 0) {
                do {
                    usleep(usecs);
                    usecs = frame_throttle->check_wait();
                } while (usecs > 0);
            } else {
                video->set_skip_frame();
            }
            video->message_frame_pass();
            video->frame_render();
        } else
            frame_throttle->skip_check();

        auto check = g_cfg.get_save_check();
        if (check) {
            if (!save_check_countdown) {
                check_save_ram();
                save_check_countdown = lround(check * fps);
            } else {
                save_check_countdown--;
            }
        }
    }
}

bool RETRO_CALLCONV retro_environment_cb(unsigned cmd, void *data) {
    if (!current_driver) return false;
    return current_driver->env_callback(cmd, data);
}

static util::LogLevel log_level_convert(retro_log_level level) {
    switch(level) {
    case RETRO_LOG_DEBUG:
        return util::LogLevel::DEBUG;
    case RETRO_LOG_INFO:
        return util::LogLevel::INFO;
    case RETRO_LOG_WARN:
        return util::LogLevel::WARN;
    case RETRO_LOG_ERROR:
        return util::LogLevel::ERROR;
    default:
        return util::LogLevel::TRACE;
    }
}

void RETRO_CALLCONV log_printf(enum retro_log_level level, const char *fmt, ...) {
#if defined(NDEBUG) || !defined(LIBRETRO_DEBUG_LOG)
    if (level >= RETRO_LOG_DEBUG)
        return;
#endif
    char s[1024];
    va_list l;
    va_start(l, fmt);
    vsnprintf(s, 1024, fmt, l);
    va_end(l);
    util::vlogMessage(log_level_convert(level), "{}", s);
}

static void RETRO_CALLCONV retro_video_refresh_cb(const void *data, unsigned width, unsigned height, size_t pitch) {
    if (!data) return;
    current_driver->get_video()->render(data, (int)width, (int)height, pitch);
}

static void RETRO_CALLCONV retro_audio_sample_cb(int16_t left, int16_t right) {
    int16_t samples[2] = {left, right};
    current_driver->get_audio()->write_samples(samples, 2);
}

static size_t RETRO_CALLCONV retro_audio_sample_batch_cb(const int16_t *data, size_t frames) {
    current_driver->get_audio()->write_samples(data, frames * 2);
    return frames;
}

static void RETRO_CALLCONV retro_input_poll_cb() {
    current_driver->get_input()->input_poll();
}

static int16_t RETRO_CALLCONV retro_input_state_cb(unsigned port, unsigned device, unsigned index, unsigned id) {
    return current_driver->get_input()->input_state(port, device, index, id);
}

static bool RETRO_CALLCONV retro_set_rumble_state_cb(unsigned port, enum retro_rumble_effect effect, uint16_t strength) {
    (void)port;
    (void)effect;
    (void)strength;
    return false;
}

bool driver_base::load_game(const std::string &path) {
    retro_game_info info = {};
    info.path = path.c_str();
    if (!need_fullpath) {
        if (!helper::read_file(path, game_data)) {
            LOG(ERROR, "Unable to load file {}", path);
            return false;
        }
        info.data = &game_data[0];
        info.size = game_data.size();
    }
    serialization_quirks = 0;
    if (!core->retro_load_game(&info)) {
        LOG(ERROR, "The core was unable to load {}", path);
        return false;
    }

    game_path = path;
    post_load();
    return true;
}

bool driver_base::load_game_from_mem(const std::string &path, const std::string &ext, const std::vector<uint8_t> &data) {
    retro_game_info info = {};
    if (!need_fullpath) {
        game_data.assign(data.begin(), data.end());
        info.path = path.c_str();
        info.data = &game_data[0];
        info.size = game_data.size();
    } else {
        std::string basename = get_base_name(path);
        temp_file = g_cfg.get_store_dir() + PATH_SEPARATOR_CHAR "tmp";
        helper::mkdir(temp_file);
        temp_file = temp_file + PATH_SEPARATOR_CHAR + basename + "." + ext;
        if (!helper::write_file(temp_file, data)) {
            remove(temp_file.c_str());
            temp_file.clear();
            return false;
        }
        info.path = temp_file.c_str();
    }
    if (!core->retro_load_game(&info)) {
        LOG(ERROR, "The core was unable to load {}", path);
        return false;
    }

    game_path = path;
    post_load();
    return true;
}

void driver_base::unload_game() {
    shutdown_driver = false;
    check_save_ram();
    core->retro_unload_game();
    audio->stop();
    video->deinit_hw_renderer();
    unload();

    if (!temp_file.empty()) {
        remove(temp_file.c_str());
        temp_file.clear();
    }

    game_path.clear();
    game_base_name.clear();
    game_save_path.clear();
    game_rtc_path.clear();
    save_data.clear();
    rtc_data.clear();
    check_save_progress = 0;
    check_rtc_progress = 0;

    serialization_quirks = 0;
}

void driver_base::reset() {
    core->retro_reset();
    audio->reset();
}

static bool camera_start_dummy() { return false; }
static void camera_stop_dummy() {}

bool driver_base::env_callback(unsigned cmd, void *data) {
    switch (cmd) {
        case RETRO_ENVIRONMENT_SET_ROTATION:
            break;
        case RETRO_ENVIRONMENT_GET_OVERSCAN:
            *(bool*)data = false;
            return true;
        case RETRO_ENVIRONMENT_GET_CAN_DUPE:
            *(bool*)data = true;
            return true;
        case RETRO_ENVIRONMENT_SET_MESSAGE: {
            const auto *msg = (const retro_message*)data;
            video->add_message(msg->msg, msg->frames);
            return true;
        }
        case RETRO_ENVIRONMENT_SHUTDOWN:
            shutdown_driver = true;
            return true;
        case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL:
            return true;
        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
            *(const char**)data = system_dir.c_str();
            return true;
        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: {
            auto new_format = (unsigned)*(const enum retro_pixel_format *)data;
            if (new_format != pixel_format) {
                pixel_format = new_format;
                video->game_resolution_changed((int)base_width, (int)base_height, (int)max_width, (int)max_height, pixel_format);
            }
            return true;
        }
        case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS: {
            const auto *inp = (const retro_input_descriptor*)data;
            while (inp->description != nullptr) {
                input->add_button_desc(inp->port, inp->device, inp->index, inp->id, inp->description);
                ++inp;
            }
            return true;
        }
        case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK: {
            keyboard_callback = new retro_keyboard_callback;
            memcpy(keyboard_callback, data, sizeof(retro_keyboard_callback));
            return true;
        }
        case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE: {
            disk_control_callback = new retro_disk_control_ext_callback;
            memset(disk_control_callback, 0, sizeof(retro_disk_control_ext_callback));
            memcpy(disk_control_callback, data, sizeof(retro_disk_control_callback));
            return true;
        }
        case RETRO_ENVIRONMENT_SET_HW_RENDER: {
            if (base_width == 0) {
                init_system_av_info();
                video->set_aspect_ratio(aspect_ratio);
                video->game_resolution_changed((int)base_width, (int)base_height, (int)max_width, (int)max_height, pixel_format);
            }
            auto hwr = (struct retro_hw_render_callback*)data;
            return video->init_hw_renderer(hwr);
        }
        case RETRO_ENVIRONMENT_GET_VARIABLE: {
            variables->set_variables_updated(false);
            auto *var = (retro_variable *)data;
            auto *vari = variables->get_variable(var->key);
            if (vari) {
                var->value = vari->options[vari->curr_index].first.c_str();
                return true;
            }
            return false;
        }
        case RETRO_ENVIRONMENT_SET_VARIABLES: {
            const auto *vars = (const retro_variable*)data;
            variables->load_variables(vars);
            variables->load_variables_from_cfg(core_cfg_path);
            return true;
        }
        case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:
            *(bool*)data = variables->get_variables_updated();
            return true;
        case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME:
            support_no_game = *(bool*)data;
            return true;
        case RETRO_ENVIRONMENT_GET_LIBRETRO_PATH:
            *(const char**)data = nullptr;
            return true;
        case RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK:
        case RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK:
            break;
        case RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE: {
            auto *ri = (retro_rumble_interface*)data;
            ri->set_rumble_state = retro_set_rumble_state_cb;
            return true;
        }
        case RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES:
            *(uint64_t*)data = (1ULL << RETRO_DEVICE_JOYPAD) | (1ULL << RETRO_DEVICE_ANALOG);
            return true;
        case RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE:
            break;
        case RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE: {
            auto *cam = (retro_camera_callback*)data;
            cam->start = &camera_start_dummy;
            cam->stop = &camera_stop_dummy;
            return true;
        }
        case RETRO_ENVIRONMENT_GET_LOG_INTERFACE: {
            ((retro_log_callback*)data)->log = log_printf;
            return true;
        }
        case RETRO_ENVIRONMENT_GET_PERF_INTERFACE: {
            libretro_get_perf_callback((retro_perf_callback*)data);
            return true;
        }
        case RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE:
        case RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY:
            break;
        case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
            *(const char**)data = core_save_dir.empty() ? nullptr : core_save_dir.c_str();
            return true;
        case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO: {
            const auto *info = (const struct retro_system_av_info *)data;
            bool restart_audio = false;
            if (fps != info->timing.fps) {
                fps = info->timing.fps;
                restart_audio = true;
            }
            if (sample_rate != info->timing.sample_rate) {
                sample_rate = info->timing.sample_rate;
                restart_audio = true;
            }
            if (restart_audio) {
                audio->stop();
                audio->start(g_cfg.get_mono_audio(), info->timing.sample_rate, g_cfg.get_sample_rate(), info->timing.fps);
            }
            bool resolution_changed = false;
            if (base_width != info->geometry.base_width) {
                base_width = info->geometry.base_width;
                resolution_changed = true;
            }
            if (base_height != info->geometry.base_height) {
                base_height = info->geometry.base_height;
                resolution_changed = true;
            }
            if (max_width != info->geometry.max_width) {
                max_width = info->geometry.max_width;
                resolution_changed = true;
            }
            if (max_height != info->geometry.max_height) {
                max_height = info->geometry.max_height;
                resolution_changed = true;
            }
            if (aspect_ratio != info->geometry.aspect_ratio) {
                aspect_ratio = info->geometry.aspect_ratio;
                video->set_aspect_ratio(aspect_ratio);
                resolution_changed = true;
            }
            if (resolution_changed) {
                video->set_aspect_ratio(aspect_ratio);
                video->game_resolution_changed((int)base_width, (int)base_height, (int)max_width, (int)max_height, pixel_format);
            }
            return true;
        }
        case RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK:
        case RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO:
            break;
        case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO: {
            // const auto *info = (const retro_controller_info*)data;
            return true;
        }
        case RETRO_ENVIRONMENT_SET_MEMORY_MAPS: {
            const auto *memmap = (const retro_memory_map*)data;
            for (unsigned i = 0; i < memmap->num_descriptors; ++i) {
                /* TODO: store info of memory map for future use */
            }
            return true;
        }
        case RETRO_ENVIRONMENT_SET_GEOMETRY: {
            const auto *geometry = (const retro_game_geometry*)data;
            bool resolution_changed = false;
            if (base_width != geometry->base_width) {
                base_width = geometry->base_width;
                resolution_changed = true;
            }
            if (base_height != geometry->base_height) {
                base_height = geometry->base_height;
                resolution_changed = true;
            }
            if (max_width != geometry->max_width) {
                max_width = geometry->max_width;
                resolution_changed = true;
            }
            if (max_height != geometry->max_height) {
                max_height = geometry->max_height;
                resolution_changed = true;
            }
            if (aspect_ratio != geometry->aspect_ratio) {
                aspect_ratio = geometry->aspect_ratio;
                video->set_aspect_ratio(aspect_ratio);
                resolution_changed = true;
            }
            if (resolution_changed) {
                video->set_aspect_ratio(aspect_ratio);
                video->game_resolution_changed((int)base_width, (int)base_height, (int)max_width, (int)max_height, pixel_format);
            }
            return true;
        }
        case RETRO_ENVIRONMENT_GET_USERNAME:
            *(const char**)data = "sdlretro";
            return true;
        case RETRO_ENVIRONMENT_GET_LANGUAGE:
            *(unsigned*)data = g_cfg.get_language();
            return true;
        case RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER:
        {
            /*
            auto *fb = (retro_framebuffer*)data;
            fb->data = video->get_framebuffer(&fb->width, &fb->height, &fb->pitch, (int*)&fb->format);
            if (fb->data)
                return true;
            */
            return false;
        }
        case RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE:
            break;
        case RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS:
            support_achivements = data == nullptr || *(bool*)data;
            return true;
        case RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE:
            /* for vulkan use only currently, so ignore it */
            return false;
        case RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS: {
            serialization_quirks = *(uint64_t*)data;
            return true;
        }
        case RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT:
            break;
        case RETRO_ENVIRONMENT_GET_VFS_INTERFACE: {
            auto *info = (struct retro_vfs_interface_info *)data;
            if (info->required_interface_version > 3) return false;
            info->iface = &libretro::vfs_interface;
            return true;
        }
        case RETRO_ENVIRONMENT_GET_LED_INTERFACE:
            break;
        case RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE:
            *(int*)data = 3;
            return true;
        case RETRO_ENVIRONMENT_GET_MIDI_INTERFACE:
        case RETRO_ENVIRONMENT_GET_FASTFORWARDING:
        case RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE:
            break;
        case RETRO_ENVIRONMENT_GET_INPUT_BITMASKS:
            if (data) *(bool*)data = true;
            return true;
        case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION:
            *(unsigned*)data = RETRO_API_VERSION;
            return true;
        case RETRO_ENVIRONMENT_SET_CORE_OPTIONS: {
            variables->load_variables((const retro_core_option_definition*)data);
            variables->load_variables_from_cfg(core_cfg_path);
            return true;
        }
        case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL: {
            variables->load_variables((const retro_core_options_intl*)data);
            variables->load_variables_from_cfg(core_cfg_path);
            return true;
        }
        case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY: {
            const auto *opt = (const retro_core_option_display*)data;
            variables->set_variable_visible(opt->key, opt->visible);
            return true;
        }
        case RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER: {
            switch (video->get_renderer_type()) {
            case 1:
                *(unsigned *)data = RETRO_HW_CONTEXT_OPENGL_CORE;
                return true;
            case 2:
                *(unsigned *)data = RETRO_HW_CONTEXT_OPENGLES3;
                return true;
            default:
                break;
            }
            return false;
        }
        case RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION: {
            *(unsigned*)data = 1;
            return true;
        }
        case RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE: {
            disk_control_callback = new retro_disk_control_ext_callback;
            memcpy(disk_control_callback, data, sizeof(retro_disk_control_ext_callback));
            return true;
        }
        case RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION:
            *(unsigned*)data = 1;
            return true;
        case RETRO_ENVIRONMENT_SET_MESSAGE_EXT: {
            auto *msg_ext = (const struct retro_message_ext*)data;
            if (msg_ext->target & RETRO_MESSAGE_TARGET_OSD) {
                // TODO: deal with msg_ext->type as well as msg_ext->progress
                video->add_message(msg_ext->msg, msg_ext->duration * fps);
            }
            if (msg_ext->target & RETRO_MESSAGE_TARGET_LOG) {
                log_printf(msg_ext->level, "%s", msg_ext->msg);
            }
            return true;
        }
        case RETRO_ENVIRONMENT_GET_INPUT_MAX_USERS: {
            // TODO: how many users do we support actually?
            *(unsigned*)data = 2;
            return true;
        }
        default:
            break;
    }
    if (cmd & RETRO_ENVIRONMENT_EXPERIMENTAL) {
        LOG(INFO, "Unhandled env: 0x{0:X} (EXPERIMENTAL)", (cmd & 0xFFFFU));
    } else {
        LOG(INFO, "Unhandled env: 0x{0:X}", (cmd & 0xFFFFU));
    }
    return false;
}

void driver_base::save_variables_to_cfg() {
    variables->save_variables_to_cfg(core_cfg_path);
}

bool driver_base::load_core(const std::string &path) {
    core = core_load(path.c_str());
    if (!core) return false;

    current_driver = this;
    input->clear_button_desc();

    retro_system_info sysinfo = {};
    core->retro_get_system_info(&sysinfo);
    library_name = sysinfo.library_name;
    library_version = sysinfo.library_version;
    need_fullpath = sysinfo.need_fullpath;
    std::string name = sysinfo.library_name;
    lowered_string(name);
    core_cfg_path = g_cfg.get_config_dir() + PATH_SEPARATOR_CHAR + "cores";
    helper::mkdir(core_cfg_path);
    core_cfg_path += PATH_SEPARATOR_CHAR + name + ".json";
    core_save_dir = save_dir + PATH_SEPARATOR_CHAR + name;
    helper::mkdir(core_save_dir);

    init_internal();
    return true;
}

bool driver_base::init_internal() {
    if (inited) return true;

    if (!init()) {
        return false;
    }

    shutdown_driver = false;
    core->retro_set_environment(retro_environment_cb);

    core->retro_init();

    core->retro_set_video_refresh(retro_video_refresh_cb);
    core->retro_set_audio_sample(retro_audio_sample_cb);
    core->retro_set_audio_sample_batch(retro_audio_sample_batch_cb);
    core->retro_set_input_poll(retro_input_poll_cb);
    core->retro_set_input_state(retro_input_state_cb);

    inited = true;
    return true;
}

void driver_base::deinit_internal() {
    if (!inited) return;

    core->retro_deinit();

    /* reset all variables to default value */
    library_name.clear();
    library_version.clear();
    need_fullpath = false;

    pixel_format = 0;
    support_no_game = false;

    base_width = 0;
    base_height = 0;
    max_width = 0;
    max_height = 0;
    aspect_ratio = 0.f;

    game_data.clear();

    variables->reset();

    inited = false;
}

void driver_base::check_save_ram() {
    check_single_ram(RETRO_MEMORY_SAVE_RAM, save_data, game_save_path, check_save_progress);
    check_single_ram(RETRO_MEMORY_RTC, rtc_data, game_rtc_path, check_rtc_progress);
}

void driver_base::check_single_ram(unsigned int id, std::vector<uint8_t> &data, const std::string &filename, size_t &pos) {
    size_t size = core->retro_get_memory_size(id);
    if (size) {
        void *ram = core->retro_get_memory_data(id);
        if (size != data.size()) {
            data.resize(size, 0);
            if (size <= pos) pos = 0;
        }
        size_t check_size = (pos + 65536) > size ? size - pos : 65536;
        LOG(TRACE, "Checking RAM block {}: from 0x{:x}, length 0x{:x}/0x{:x}", id, pos, check_size, size);
        if (memcmp((uint8_t*)ram + pos, data.data() + pos, check_size) == 0) {
            pos += check_size;
            if (pos >= size) pos = 0;
            return;
        }
        pos = 0;
        LOG(TRACE, "RAM changed, saving to {}", filename);
        data.assign((uint8_t*)ram, (uint8_t*)ram + size);
        helper::write_file(filename, data);
    }
}

void driver_base::post_load() {
    init_system_av_info();

    video->inited_hw_renderer();

    game_base_name = get_base_name(game_path);
    game_save_path = (core_save_dir.empty() ? "" : (core_save_dir + PATH_SEPARATOR_CHAR)) + game_base_name + ".sav";
    game_rtc_path = (core_save_dir.empty() ? "" : (core_save_dir + PATH_SEPARATOR_CHAR)) + game_base_name + ".rtc";

    helper::read_file(game_save_path, save_data);
    helper::read_file(game_rtc_path, rtc_data);

    if (!save_data.empty()) {
        size_t sz = core->retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);
        if (sz > save_data.size()) sz = save_data.size();
        if (sz) memcpy(core->retro_get_memory_data(RETRO_MEMORY_SAVE_RAM), save_data.data(), sz);
    }
    if (!rtc_data.empty()) {
        size_t sz = core->retro_get_memory_size(RETRO_MEMORY_RTC);
        if (sz > rtc_data.size()) sz = rtc_data.size();
        if (sz) memcpy(core->retro_get_memory_data(RETRO_MEMORY_RTC), rtc_data.data(), sz);
    }

    audio->start(g_cfg.get_mono_audio(), sample_rate, g_cfg.get_sample_rate(), fps);
    frame_throttle->reset(fps);
    core->retro_set_controller_port_device(0, RETRO_DEVICE_JOYPAD);
    video->set_aspect_ratio(aspect_ratio);
    video->game_resolution_changed((int)base_width, (int)base_height, (int)max_width, (int)max_height, pixel_format);

    char library_message[256];
    snprintf(library_message, 256, "Loaded core: %s"_i18n, library_name.c_str());
    video->add_message(library_message, lround(fps * 5));
}

void driver_base::init_system_av_info() {
    if (base_width != 0) {
        return;
    }
    retro_system_av_info av_info = {};
    core->retro_get_system_av_info(&av_info);
    base_width = av_info.geometry.base_width;
    base_height = av_info.geometry.base_height;
    max_width = av_info.geometry.max_width;
    max_height = av_info.geometry.max_height;
    aspect_ratio = av_info.geometry.aspect_ratio;
    fps = av_info.timing.fps;
    sample_rate = av_info.timing.sample_rate;
}

}
