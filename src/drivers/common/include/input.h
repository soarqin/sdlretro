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

class input {
public:
    void add_button(unsigned port, unsigned device, unsigned index, unsigned id, const std::string &desc);

private:
    std::vector<input_port_t> ports;
};

}
