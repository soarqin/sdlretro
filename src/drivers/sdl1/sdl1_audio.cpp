#include "sdl1_audio.h"

#include <SDL.h>

#include <memory.h>

namespace drivers {

static void audio_callback(void *userdata, Uint8 *stream, int len) {
    auto *audio = static_cast<sdl1_audio*>(userdata);
    audio->read_samples(reinterpret_cast<int16_t*>(stream), len / 2);
}

bool sdl1_audio::open_audio(unsigned buffer_size) {
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

void sdl1_audio::close_audio() {
    SDL_CloseAudio();
}

}
