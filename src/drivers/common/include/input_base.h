#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>

namespace drivers {

struct output_button_t {
    uint16_t id;
    uint8_t index;

    uint8_t port;

    std::string description;
};

struct output_port_t {
    bool available = false;
    bool enabled = false;

    uint8_t device;
    std::map<uint32_t, output_button_t> buttons;

    int16_t states = 0;
    int16_t analog_axis[2][2] = {};
};

struct input_button_t {
    uint64_t value;
    std::string name;
};

class input_base {
public:
    enum input_mode {
        mode_game,  /* game */
        mode_menu,  /* menu */
        mode_input, /* wait for input, used for settings */
    };

    virtual ~input_base() = default;

    /* virtual method for input poll callback use */
    virtual void input_poll() = 0;

    /* virtual method called on gamepad connected */
    virtual void port_connected(int index) = 0;

    /* virtual method called on gamepad disconnected */
    virtual void port_disconnected(int device_id) = 0;

    /* virtual methods to translate name <-> code */
    virtual void get_input_name(uint64_t input, std::string &device_name, std::string &name) const = 0;
    virtual uint64_t get_input_from_name(const std::string &device_name, const std::string &name) const = 0;

    inline uint64_t get_last_input() const {
        return last_input;
    }

    inline void clear_last_input() {
        last_input = 0;
    }

    inline void on_key(uint16_t id, bool pressed) {
        on_input(id, pressed);
    }
    inline void on_mouse(uint16_t id, bool pressed) {
        on_input(1024 + id, pressed);
    }
    inline void on_joybtn(int device_id, uint16_t id, bool pressed) {
        on_input((static_cast<uint64_t>(device_id) << 16) | static_cast<uint64_t>(id), pressed);
    }

    int16_t input_state(unsigned port, unsigned device, unsigned index, unsigned id);

    void add_button_desc(uint8_t port, uint8_t device, uint8_t index, uint16_t id, const std::string &desc);
    void clear_button_desc();
    void clear_menu_button_desc();

    inline void map_key(uint16_t from_id, uint8_t to_port, uint16_t to_id) {
        add_mapping(from_id, to_port, to_id);
    }

    inline void map_mouse(uint16_t from_id, uint8_t to_port, uint16_t to_id) {
        add_mapping(1024 + from_id, to_port, to_id);
    }

    inline void map_joybtn(uint32_t from_device_id, uint16_t from_id, uint8_t to_port, uint16_t to_id) {
        add_mapping((static_cast<uint64_t>(from_device_id) << 16) | static_cast<uint64_t>(from_id), to_port, to_id);
    }

    inline int16_t get_menu_pad_states() const { return port_menu.states; }
    inline void set_input_mode(input_mode m) { mode = m; }

    void foreach_mapping(std::function<void(const output_button_t &output, const input_button_t &input)> cb) const;

    void add_mapping(uint64_t from, uint8_t to_port, uint16_t to_id);
    void remove_mapping(uint8_t to_port, uint16_t to_id);

    void save_to_cfg();
    void load_from_cfg();

private:
    void on_input(uint64_t id, bool pressed);

protected:
    static inline uint32_t button_packed_value(uint8_t index, uint16_t id) {
        return static_cast<uint32_t>(id) | (static_cast<uint32_t>(index) << 16);
    }

protected:
    std::map<uint64_t, output_button_t*> game_mapping, menu_mapping;
    std::map<uint64_t, uint64_t> rev_game_mapping, rev_menu_mapping;

    std::vector<output_port_t> ports;
    output_port_t port_menu {};

    input_mode mode = mode_game;
    uint64_t last_input = 0;
};

}
