#include "sdl2_audio.h"

#include <SDL.h>

namespace drivers {

void sdl2_audio::audio_callback(void *userdata, uint8_t *stream, int len) {
    auto *audio = static_cast<sdl2_audio*>(userdata);
    audio->read_samples(reinterpret_cast<int16_t*>(stream), len / 2);
}

bool sdl2_audio::open(unsigned buffer_size) {
    if (!SDL_WasInit(SDL_INIT_AUDIO))
        SDL_InitSubSystem(SDL_INIT_AUDIO);
    SDL_AudioSpec spec = {}, obtained = {};
    spec.callback = audio_callback;
    spec.freq = static_cast<int>(output_sample_rate);
    spec.format = AUDIO_S16SYS;
    spec.channels = mono_audio ? 1 : 2;
    spec.samples = buffer_size;
    spec.userdata = this;
    if ((device_id = SDL_OpenAudioDevice(nullptr, 0, &spec, &obtained, SDL_AUDIO_ALLOW_SAMPLES_CHANGE | SDL_AUDIO_ALLOW_FREQUENCY_CHANGE)) < 2) return false;
    output_sample_rate = obtained.freq;

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

}
