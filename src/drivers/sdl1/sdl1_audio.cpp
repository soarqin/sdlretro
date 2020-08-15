#include "sdl1_audio.h"

#include <SDL.h>

namespace drivers {

void sdl1_audio::audio_callback(void *userdata, uint8_t *stream, int len) {
    auto *audio = static_cast<sdl1_audio*>(userdata);
    audio->read_samples(reinterpret_cast<int16_t*>(stream), len / 2);
}

bool sdl1_audio::open(unsigned buffer_size) {
    if (!SDL_WasInit(SDL_INIT_AUDIO))
        SDL_InitSubSystem(SDL_INIT_AUDIO);
    SDL_AudioSpec spec = {}, obtained = {};
    spec.callback = audio_callback;
    spec.freq = static_cast<int>(output_sample_rate);
    spec.format = AUDIO_S16SYS;
    spec.channels = mono_audio ? 1 : 2;
    spec.samples = buffer_size;
    spec.userdata = this;
    if (SDL_OpenAudio(&spec, &obtained) != 0) return false;
    output_sample_rate = static_cast<unsigned>(obtained.freq);

    SDL_PauseAudio(0);
    return true;
}

void sdl1_audio::close() {
    output_sample_rate = 0;
    SDL_CloseAudio();
}

void sdl1_audio::pause(bool b) {
    SDL_PauseAudio(b);
}

}
