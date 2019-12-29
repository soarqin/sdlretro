#pragma once

#define RETRO_IMPORT_SYMBOLS
#include "libretro.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct retro_core_t {
    void *symlib;

    /* Library global initialization/deinitialization. */
    void (*retro_init)(void);
    void (*retro_deinit)(void);

    /* Sets callbacks. retro_set_environment() is guaranteed to be called
     * before retro_init().
     *
     * The rest of the set_* functions are guaranteed to have been called
     * before the first call to retro_run() is made. */
    void (*retro_set_environment)(retro_environment_t);
    void (*retro_set_video_refresh)(retro_video_refresh_t);
    void (*retro_set_audio_sample)(retro_audio_sample_t);
    void (*retro_set_audio_sample_batch)(retro_audio_sample_batch_t);
    void (*retro_set_input_poll)(retro_input_poll_t);
    void (*retro_set_input_state)(retro_input_state_t);

    /* Must return RETRO_API_VERSION. Used to validate ABI compatibility
     * when the API is revised. */
    unsigned (*retro_api_version)(void);

    /* Gets statically known system info. Pointers provided in *info
     * must be statically allocated.
     * Can be called at any time, even before retro_init(). */
    void (*retro_get_system_info)(struct retro_system_info *info);

    /* Gets information about system audio/video timings and geometry.
     * Can be called only after retro_load_game() has successfully completed.
     * NOTE: The implementation of this function might not initialize every
     * variable if needed.
     * E.g. geom.aspect_ratio might not be initialized if core doesn't
     * desire a particular aspect ratio. */
    void (*retro_get_system_av_info)(struct retro_system_av_info *info);

    /* Sets device to be used for player 'port'.
     * By default, RETRO_DEVICE_JOYPAD is assumed to be plugged into all
     * available ports.
     * Setting a particular device type is not a guarantee that libretro cores
     * will only poll input based on that particular device type. It is only a
     * hint to the libretro core when a core cannot automatically detect the
     * appropriate input device type on its own. It is also relevant when a
     * core can change its behavior depending on device type.
     *
     * As part of the core's implementation of retro_set_controller_port_device,
     * the core should call RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS to notify the
     * frontend if the descriptions for any controls have changed as a
     * result of changing the device type.
     */
    void (*retro_set_controller_port_device)(unsigned port, unsigned device);

    /* Resets the current game. */
    void (*retro_reset)(void);

    /* Runs the game for one video frame.
     * During retro_run(), input_poll callback must be called at least once.
     *
     * If a frame is not rendered for reasons where a game "dropped" a frame,
     * this still counts as a frame, and retro_run() should explicitly dupe
     * a frame if GET_CAN_DUPE returns true.
     * In this case, the video callback can take a NULL argument for data.
     */
    void (*retro_run)(void);

    /* Returns the amount of data the implementation requires to serialize
     * internal state (save states).
     * Between calls to retro_load_game() and retro_unload_game(), the
     * returned size is never allowed to be larger than a previous returned
     * value, to ensure that the frontend can allocate a save state buffer once.
     */
    size_t (*retro_serialize_size)(void);

    /* Serializes internal state. If failed, or size is lower than
     * retro_serialize_size(), it should return false, true otherwise. */
    bool (*retro_serialize)(void *data, size_t size);
    bool (*retro_unserialize)(const void *data, size_t size);

    void (*retro_cheat_reset)(void);
    void (*retro_cheat_set)(unsigned index, bool enabled, const char *code);

    /* Loads a game.
     * Return true to indicate successful loading and false to indicate load failure.
     */
    bool (*retro_load_game)(const struct retro_game_info *game);

    /* Loads a "special" kind of game. Should not be used,
     * except in extreme cases. */
    bool (*retro_load_game_special)(
        unsigned game_type,
        const struct retro_game_info *info, size_t num_info
    );

    /* Unloads the currently loaded game. Called before retro_deinit(void). */
    void (*retro_unload_game)(void);

    /* Gets region of game. */
    unsigned (*retro_get_region)(void);

    /* Gets region of memory. */
    void *(*retro_get_memory_data)(unsigned id);
    size_t (*retro_get_memory_size)(unsigned id);

} retro_core_t;

retro_core_t *core_load(const char *path);
void core_unload(retro_core_t *core);

#ifdef __cplusplus
}
#endif
