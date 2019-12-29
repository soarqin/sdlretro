#include "input.h"

namespace drivers {

void input::add_button(unsigned port, unsigned device, unsigned index, unsigned id, const std::string &desc) {
    input_button_t bt = { true, port, device, index, id, desc };
    if (port >= ports.size()) ports.resize(port + 1);
    auto &p = ports[port];
    if (id >= p.buttons.size()) p.buttons.resize(id + 1);
    p.buttons[id] = bt;
}

}
