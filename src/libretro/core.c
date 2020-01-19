#include "include/core.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "dlfcn_compat.h"

#define SYMLOAD(x) do { \
   void *func = dlsym(lib, #x); \
   memcpy(&core->x, &func, sizeof(func)); \
   if (core->x == NULL) { fprintf(stderr, "Failed to load symbol: \"%s\"\n", #x); exit(1); } \
} while (0)

retro_core_t *core_load(const char *path) {
    retro_core_t *core;
    void *lib = dlopen(path, RTLD_LAZY);
    if (!lib) {
        return NULL;
    }
    core = (retro_core_t*)calloc(1, sizeof(retro_core_t));
    if (!core) {
        dlclose(lib);
        return NULL;
    }
    SYMLOAD(retro_init);
    SYMLOAD(retro_deinit);
    SYMLOAD(retro_set_environment);
    SYMLOAD(retro_set_video_refresh);
    SYMLOAD(retro_set_audio_sample);
    SYMLOAD(retro_set_audio_sample_batch);
    SYMLOAD(retro_set_input_poll);
    SYMLOAD(retro_set_input_state);
    SYMLOAD(retro_api_version);
    SYMLOAD(retro_get_system_info);
    SYMLOAD(retro_get_system_av_info);
    SYMLOAD(retro_set_controller_port_device);
    SYMLOAD(retro_reset);
    SYMLOAD(retro_run);
    SYMLOAD(retro_serialize_size);
    SYMLOAD(retro_serialize);
    SYMLOAD(retro_unserialize);
    SYMLOAD(retro_cheat_reset);
    SYMLOAD(retro_cheat_set);
    SYMLOAD(retro_load_game);
    SYMLOAD(retro_load_game_special);
    SYMLOAD(retro_unload_game);
    SYMLOAD(retro_get_region);
    SYMLOAD(retro_get_memory_data);
    SYMLOAD(retro_get_memory_size);
    core->symlib = lib;

    return core;
}

void core_unload(retro_core_t *core) {
    dlclose(core->symlib);
    memset(core, 0, sizeof(retro_core_t));
    free(core);
}
