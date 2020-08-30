#pragma once

#include <string>
#include <vector>
#include <map>
#include <tuple>

namespace drivers {

struct output_button_t {
    uint16_t id;
    uint8_t index;

    uint8_t port;

    bool available;

    std::string description;
};

struct input_button_t {
    // key/joybutton id
    //  for keyboard/mouse:
    //    0-1023   key scancode
    //    1024-        mouse input (1024 + mousebtn id)
    uint16_t id;
    // 0   keyboard/mouse
    // 1-  joypad port n
    uint8_t port;
};

struct input_port_t {
    bool available = false;
    bool enabled = false;

    uint8_t device;
    std::map<uint32_t, output_button_t> buttons;

    int16_t states = 0;
    int16_t analog_axis[2][2] = {};
};

class input_base {
public:
    virtual ~input_base() = default;

    /* virtual method for callback use */
    virtual void input_poll() = 0;
    virtual int16_t input_state(unsigned port, unsigned device, unsigned index, unsigned id);

    void add_button_desc(uint8_t port, uint8_t device, uint8_t index, uint16_t id, const std::string &desc);
    void clear_button_desc();
    inline void map_key(uint16_t from_id, uint8_t to_port, uint16_t to_id) {
        add_mapping(from_id, to_port, to_id);
    }

    inline void map_mouse(uint16_t from_id, uint8_t to_port, uint16_t to_id) {
        add_mapping(1024 + from_id, to_port, to_id);
    }

    inline void map_joybtn(uint8_t from_port, uint16_t from_id, uint8_t to_port, uint16_t to_id) {
        add_mapping(button_packed_value(from_port + 1, from_id), to_port, to_id);
    }

    inline int16_t get_pad_states(unsigned index) { return ports[index].states; }

    inline void on_key(uint16_t id, bool pressed) {
        on_input(id, pressed);
    }
    inline void on_mouse(uint16_t id, bool pressed) {
        on_input(1024 + id, pressed);

    }
    inline void on_joybtn(uint8_t port, uint16_t id, bool pressed) {
        on_input(button_packed_value(port + 1, id), pressed);
    }

private:
    void add_mapping(uint32_t from, uint8_t to_port, uint16_t to_id);
    void on_input(uint32_t id, bool pressed);

protected:
    static inline uint32_t button_packed_value(uint8_t index, uint16_t id) {
        return static_cast<uint32_t>(id) | (static_cast<uint32_t>(index) << 16);
    }

protected:
    std::map<uint32_t, std::tuple<uint8_t, uint32_t, output_button_t*>> key_mapping;

    std::vector<input_port_t> ports = {{false, true}};
};

}
