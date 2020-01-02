#pragma once

#include <string>
#include <vector>

namespace drivers {

struct input_button_t {
    bool available = false;

    unsigned port = 0;
    unsigned device = 0;
    unsigned index = 0;
    unsigned id = 0;

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

    virtual void init() = 0;
    virtual void deinit() = 0;

    /* virtual method for callback use */
    virtual void input_poll() = 0;
    virtual int16_t input_state(unsigned port, unsigned device, unsigned index, unsigned id);

protected:
    int16_t pad_states = 0;

private:
    std::vector<input_port_t> ports;
};

}
