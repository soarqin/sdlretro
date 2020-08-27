#pragma once

#include <string>
#include <vector>
#include <map>
#include <tuple>

namespace drivers {

struct input_button_t {
    union {
        struct {
            uint16_t id;
            uint8_t index;
            uint8_t device;
        };
        uint32_t value;
    };

    bool available;

    std::string description;
};

union input_key_t {
    // combined value
    uint32_t value;
    struct {
        // key/joybutton id
        //  for keyboard/mouse:
        //    0-(SDL_NUM_SCANCODES-1)   key scancode
        //    SDL_NUM_SCANCODES-        mouse input (SDL_NUM_SCANCODES + mousebtn id)
        uint16_t id;
        // 0   keyboard/mouse
        // 1-  joypad port n
        uint8_t port;
    };
};

struct input_port_t {
    bool available = false;
    bool enabled = false;

    std::map<uint32_t, input_button_t> buttons;

    int16_t states = 0;
    int16_t analog_axis[2][2] = {};
};

class input_base {
public:
    virtual ~input_base() = default;

    void add_button_desc(uint8_t port, uint8_t device, uint8_t index, uint16_t id, const std::string &desc);
    void map_button(uint32_t from, uint8_t to_port, uint32_t to_id);

    inline int16_t get_pad_states(unsigned index) { return ports[index].states; }

    /* virtual method for callback use */
    virtual void input_poll() = 0;
    virtual int16_t input_state(unsigned port, unsigned device, unsigned index, unsigned id);

    void on_keydown(uint16_t id);
    void on_keyup(uint16_t id);
    void on_mousedown(uint16_t id);
    void on_mouseup(uint16_t id);
    void on_joybtndown(uint8_t port, uint16_t id);
    void on_joybtnup(uint8_t port, uint16_t id);

protected:
    std::map<uint32_t, std::tuple<uint8_t, uint32_t>> key_mapping;

    std::vector<input_port_t> ports;
};

}
