#pragma once

#include <gamecontrollerdb.h>

#include <libretro.h>

#include <string>
#include <map>
#include <array>
#include <functional>

enum :uint16_t {
    RETRO_DEVICE_ID_ANALOG_LX = (1 << 8) + RETRO_DEVICE_ID_ANALOG_X,
    RETRO_DEVICE_ID_ANALOG_LY = (1 << 8) + RETRO_DEVICE_ID_ANALOG_Y,
    RETRO_DEVICE_ID_ANALOG_RX = (2 << 8) + RETRO_DEVICE_ID_ANALOG_X,
    RETRO_DEVICE_ID_ANALOG_RY = (2 << 8) + RETRO_DEVICE_ID_ANALOG_Y,
};

namespace drivers {

struct output_button_t {
    uint8_t device;
    uint16_t id;
    uint8_t index;

    uint8_t port;

    std::string description;
};

struct output_port_t {
    bool available = false;
    bool enabled = false;

    std::map<uint16_t, output_button_t> buttons;

    /* 0 for keyboard and mouse, otherwise device id for joystick */
    uint32_t device_id = 0xFFFFFFFFu;

    /* keyboard and mouse state */
    int16_t states = 0;
    int16_t analog_axis[4] = {};

    /* game controller state */
    gamecontrollerdb::ControllerState *cstate = nullptr;
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

    /* virtual method for doing post-init(load input.json first) jobs */
    virtual void post_init();

    /* virtual method for input poll callback use,
     * implement it if not handling input events. */
    virtual void input_poll() { }

    /* virtual methods to translate name <-> code */
    virtual void get_input_name(uint16_t id, std::string &device_name, std::string &name) const = 0;
    virtual uint16_t get_input_from_name(const std::string &device_name, const std::string &name) const = 0;

    inline uint64_t get_last_input() const {
        return last_input;
    }

    inline void clear_last_input() {
        last_input = 0;
    }

    int16_t input_state(unsigned port, unsigned device, unsigned index, unsigned id);

    void add_button_desc(uint8_t port, uint8_t device, uint8_t index, uint16_t id, const std::string &desc);
    void clear_button_desc();

    void set_input_mode(input_mode m);

    void foreach_km_mapping(const std::function<void(const output_button_t &output, const input_button_t &input)> &cb) const;

    std::pair<uint16_t, uint16_t> set_km_mapping(uint16_t from, uint16_t to_id);

    void save_to_cfg();
    void load_from_cfg();

    void on_km_input(uint16_t id, bool pressed);

    bool on_device_connected(uint32_t device_id, const gamecontrollerdb::GUID &guid);
    void on_device_disconnected(uint32_t device_id);
    void on_joybtn_input(uint32_t device_id, uint8_t id, bool pressed);
    void on_joyhat_input(uint32_t device_id, uint8_t id, uint8_t value);
    void on_joyaxis_input(uint32_t device_id, uint8_t id, int16_t value);

private:
    void assign_port(uint32_t device_id, uint8_t port);
    void unassign_port(uint8_t port);

private:
    std::array<output_port_t, 8> ports {};

    std::map<uint32_t, uint8_t> port_mapping;
    std::map<uint16_t, uint16_t> km_to_game_mapping;
    std::map<uint16_t, uint16_t> game_to_km_mapping;

    input_mode mode = mode_game;
    uint64_t last_input = 0;

    gamecontrollerdb::DB gcdb;
    std::map<uint32_t, gamecontrollerdb::ControllerState> gcontrollers;
};

}
