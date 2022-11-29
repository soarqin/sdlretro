#include "sdl2_audio.h"

#include <helper.h>

#include <SDL.h>

namespace drivers {

void sdl2_audio::reset() {
    audio_base::reset();
    SDL_ClearQueuedAudio(device_id);
}

bool sdl2_audio::open(unsigned buffer_size) {
    if (!SDL_WasInit(SDL_INIT_AUDIO))
        SDL_InitSubSystem(SDL_INIT_AUDIO);
    SDL_AudioSpec spec = {}, obtained = {};
    spec.freq = static_cast<int>(output_sample_rate);
    spec.format = AUDIO_S16SYS;
    spec.channels = mono_audio ? 1 : 2;
    spec.samples = buffer_size;
    if ((device_id = SDL_OpenAudioDevice(nullptr, 0, &spec, &obtained, SDL_AUDIO_ALLOW_SAMPLES_CHANGE | SDL_AUDIO_ALLOW_FREQUENCY_CHANGE)) < 2) return false;
    output_sample_rate = obtained.freq;
    max_queued_samples = output_sample_rate / 2;

    SDL_PauseAudioDevice(device_id, 0);
    return true;
}

void sdl2_audio::close() {
    output_sample_rate = 0;
    SDL_CloseAudioDevice(device_id);
}

void sdl2_audio::pause(bool b) {
    SDL_PauseAudioDevice(device_id, b);
}

void sdl2_audio::on_input(const int16_t *samples, size_t count) {
    /* check queue overflow every 250ms */
    uint64_t now = helper::get_ticks_usec_cache();
    if (now >= next_check) {
        if (SDL_GetQueuedAudioSize(device_id) > max_queued_samples) {
            SDL_ClearQueuedAudio(device_id);
        }
        next_check = now + 250000ULL;
    }
    SDL_QueueAudio(device_id, samples, count * sizeof(int16_t));
}

}
