#pragma once

#include <string>
#include <vector>
#include <map>

namespace drivers {

struct input_button_t {
    bool available;

    unsigned port;
    unsigned device;
    unsigned index;
    unsigned id;

    std::string description;
};

union input_key_t {
    // combined value
    uint32_t value;
    struct {
        // key/joybutton id
        //  for keyboard:
        //    0-(SDL_NUM_SCANCODES-1)   key scancode
        //    SDL_NUM_SCANCODES-        mouse input (SDL_NUM_SCANCODES + mousebtn id)
        uint16_t id;
        // 0   keyboard
        // 1-  joypad port n
        uint8_t port;
    };
};

struct input_port_t {
    bool available = false;
    std::vector<input_button_t> buttons;
};

class input_base {
public:
    virtual ~input_base() = default;

    void add_button(unsigned port, unsigned device, unsigned index, unsigned id, const std::string &desc);

    inline int16_t get_pad_states(unsigned index) { return pad_states[index]; }

    /* virtual method for callback use */
    virtual void input_poll() = 0;
    virtual int16_t input_state(unsigned port, unsigned device, unsigned index, unsigned id);

protected:
    bool pad_enabled[2] = {true, false};
    int16_t pad_states[2] = {};
    int16_t analog_axis[2][2][2] = {};

    // for value: (port << 8) | (joypad btn id)
    std::map<uint32_t, uint16_t> key_mapping;

private:
    std::vector<input_port_t> ports;
};

}
