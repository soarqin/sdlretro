#include "sdl1_audio.h"

#include <SDL.h>

namespace drivers {

void sdl1_audio::reset() {
    buffer.clear();
}

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

    buffer.resize(buffer_size * 8);

    SDL_PauseAudio(0);
    return true;
}

void sdl1_audio::close() {
    output_sample_rate = 0;
    SDL_CloseAudio();
    buffer.clear();
}

void sdl1_audio::on_input(const int16_t *samples, size_t count) {
    buffer.push(samples, count);
}

void sdl1_audio::read_samples(int16_t *data, size_t count) {
    if (!count) return;
    size_t read_count = buffer.pop(data, count);
    if (read_count < count) {
        memset(data + read_count, 0, (count - read_count) * sizeof(int16_t));
    }
}

void sdl1_audio::pause(bool b) {
    SDL_PauseAudio(b);
}

}
