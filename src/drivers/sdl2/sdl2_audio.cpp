#include "sdl2_audio.h"

#include <spdlog/spdlog.h>

#include <SDL.h>

namespace drivers {

void sdl2_audio::reset() {
    buffered_audio::reset();
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
    // spec.callback = audio_callback;
    // spec.userdata = this;
    if ((device_id = SDL_OpenAudioDevice(nullptr, 0, &spec, &obtained, SDL_AUDIO_ALLOW_SAMPLES_CHANGE | SDL_AUDIO_ALLOW_FREQUENCY_CHANGE)) < 2) return false;
    output_sample_rate = obtained.freq;
    max_queued_samples = output_sample_rate * 2;

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

void sdl2_audio::on_input() {
    /* too many queued samples, which indicates that a frame drop occurs,
     * we have to drop queued samples to ensure audio sync
     * TODO: move this to a timer? */
    if (SDL_GetQueuedAudioSize(device_id) > max_queued_samples) {
        SDL_ClearQueuedAudio(device_id);
    }
    int16_t samples[4096];
    size_t count = samples_count();
    while(count > 0) {
        size_t drain = count < 4096 ? count : 4096;
        read_samples(samples, drain);
        SDL_QueueAudio(device_id, samples, drain * sizeof(int16_t));
        count -= drain;
    }
}

/*
void sdl2_audio::audio_callback(void *userdata, uint8_t *stream, int len) {
    auto *audio = static_cast<sdl2_audio*>(userdata);
    audio->read_samples(reinterpret_cast<int16_t*>(stream), len / 2);
}
*/
}
