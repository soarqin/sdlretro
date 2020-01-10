#pragma once

#include <string>
#include <vector>

namespace drivers {

struct input_button_t {
    bool available;

    unsigned port;
    unsigned device;
    unsigned index;
    unsigned id;

    std::string description;
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
    int16_t analog_axis[2][3][2] = {};

private:
    std::vector<input_port_t> ports;
};

}
