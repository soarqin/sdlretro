#include "input_base.h"

#include <util.h>
#include <cfg.h>

#include <spdlog/spdlog.h>
#include <json.hpp>

using json = nlohmann::json;

namespace drivers {

void input_base::post_init() {
    gcdb.addFromString("");
    load_from_cfg();
}

int16_t input_base::input_state(unsigned port, unsigned device, unsigned index, unsigned id) {
    if (port >= ports.size()) return 0;
    auto &p = ports[port];
    if (!p.enabled) return 0;
    switch (device) {
    case RETRO_DEVICE_JOYPAD:
        if (id == RETRO_DEVICE_ID_JOYPAD_MASK) {
            return p.states;
        }
        return (p.states & (1 << id)) ? 1 : 0;
    case RETRO_DEVICE_KEYBOARD:
        return 0;
    case RETRO_DEVICE_ANALOG:
        if (index <= RETRO_DEVICE_INDEX_ANALOG_RIGHT) {
            auto axis_id = (index << 1) + id;
            if (axis_id < 4) {
                return p.analog_axis[axis_id];
            }
        } else if (index == RETRO_DEVICE_INDEX_ANALOG_BUTTON) {
            if (id == RETRO_DEVICE_ID_JOYPAD_MASK) {
                return p.states;
            }
            return (p.states & (1 << id)) ? 1 : 0;
        }
        return 0;
    default:
        return 0;
    }
}

void input_base::add_button_desc(uint8_t port, uint8_t device, uint8_t index, uint16_t id, const std::string &desc) {
    output_button_t bt = {device, id, index, port, desc};
    if (port >= ports.size()) {
        return;
    }
    auto &p = ports[port];
    if (!p.available) {
        p.available = true;
    }
    if (device == RETRO_DEVICE_ANALOG) {
        /* remap device index like this:
         *   RETRO_DEVICE_INDEX_ANALOG_BUTTON -> 0
         *   RETRO_DEVICE_INDEX_ANALOG_LEFT   -> 1
         *   RETRO_DEVICE_INDEX_ANALOG_RIGHT  -> 2
         */
        if (index < RETRO_DEVICE_INDEX_ANALOG_BUTTON) {
            ++index;
        } else if (index == RETRO_DEVICE_INDEX_ANALOG_BUTTON) {
            index = 0;
        }
    }
    auto value = static_cast<uint16_t>(id) | (static_cast<uint16_t>(index) << 8);
    p.buttons[value] = std::move(bt);
}

void input_base::clear_button_desc() {
    for (auto &p: ports) {
        p.buttons.clear();
    }
}

void input_base::save_to_cfg() {
    json j;
    auto &kj = j["keyboard/mouse"];
    for (auto &p: game_to_km_mapping) {
        auto id = p.first;

        std::string device_name, name;
        get_input_name(p.second, device_name, name);
        auto &sub = kj[std::to_string(id)];
        sub["device"] = device_name;
        sub["name"] = name;
    }
    auto filename = g_cfg.get_config_dir() + PATH_SEPARATOR_CHAR + "input.json";
    try {
        auto content = j.dump(4);
        if (!util::write_file(filename, content))
            throw std::bad_exception();
    } catch(...) {
        spdlog::error("failed to write input config to {}", filename);
    }
}

void input_base::load_from_cfg() {
    json j;
    auto filename = g_cfg.get_config_dir() + PATH_SEPARATOR_CHAR + "input.json";
    if (!util::file_exists(filename)) return;
    try {
        std::string content;
        if (!util::read_file(filename, content)) {
            throw std::bad_exception();
        }
        j = json::parse(content);
    } catch(...) {
        spdlog::error("failed to read input config from {}", filename);
        return;
    }
    auto &kj = j["keyboard/mouse"];
    if (kj.is_object()) {
        for (auto &bt: kj.items()) {
            auto btid = std::stoi(bt.key());
            auto &from = bt.value();
            auto fromid = get_input_from_name(from["device"], from["name"]);
            if (fromid) {
                set_km_mapping(fromid, btid);
            }
        }
    }
}

const char *libretro_button_name(uint16_t id) {
    auto index = id >> 8;
    switch(index) {
    case 0: {
        const char *name[16] = {
            "B",
            "Y",
            "SELECT",
            "START",
            "UP",
            "DOWN",
            "LEFT",
            "RIGHT",
            "A",
            "X",
            "L",
            "R",
            "L2",
            "R2",
            "L3",
            "R3",
        };
        auto btn_id = id & 0xFF;
        return btn_id < 16 ? name[btn_id] : "";
    }
    case 1:
    case 2: {
        const char *analog_name[2][2] = {
            {"L-Axis X", "L-Axis Y"},
            {"R-Axis X", "R-Axis Y"},
        };
        auto axis_id = id & 0xFF;
        return axis_id < 2 ? analog_name[index - 1][axis_id] : "";
        break;
    }
    default:
        return "";
    }
}

void input_base::set_input_mode(input_base::input_mode m) {
    if (mode == m) {
        return;
    }
    if (mode == mode_game || m == mode_game) {
        /* clear inputs on enter/leave menu */
        for (auto &p: ports) {
            p.states = 0;
            memset(p.analog_axis, 0, sizeof(p.analog_axis));
        }
    }
    mode = m;
}

void input_base::foreach_km_mapping(const std::function<void(const output_button_t &, const input_button_t &)> &cb) const {
    const uint16_t buttons[] = {
        RETRO_DEVICE_ID_JOYPAD_UP,
        RETRO_DEVICE_ID_JOYPAD_DOWN,
        RETRO_DEVICE_ID_JOYPAD_LEFT,
        RETRO_DEVICE_ID_JOYPAD_RIGHT,
        RETRO_DEVICE_ID_JOYPAD_A,
        RETRO_DEVICE_ID_JOYPAD_B,
        RETRO_DEVICE_ID_JOYPAD_X,
        RETRO_DEVICE_ID_JOYPAD_Y,
        RETRO_DEVICE_ID_JOYPAD_SELECT,
        RETRO_DEVICE_ID_JOYPAD_START,
        RETRO_DEVICE_ID_JOYPAD_L,
        RETRO_DEVICE_ID_JOYPAD_R,
        RETRO_DEVICE_ID_JOYPAD_L2,
        RETRO_DEVICE_ID_JOYPAD_R2,
        RETRO_DEVICE_ID_JOYPAD_L3,
        RETRO_DEVICE_ID_JOYPAD_R3,
        RETRO_DEVICE_ID_ANALOG_LX,
        RETRO_DEVICE_ID_ANALOG_LY,
        RETRO_DEVICE_ID_ANALOG_RX,
        RETRO_DEVICE_ID_ANALOG_RY,
    };
    for (auto &b: buttons) {
        auto ite = game_to_km_mapping.find(b);
        if (ite == game_to_km_mapping.end()) {
            continue;
        }
        output_button_t ob = {};
        ob.id = ite->first;
        ob.description = libretro_button_name(ob.id);
        input_button_t ib = {};
        ib.value = ite->second;
        std::string device_name;
        get_input_name(ib.value, device_name, ib.name);
        cb(ob, ib);
    }
}

std::pair<uint16_t, uint16_t> input_base::set_km_mapping(uint16_t from, uint16_t to_id) {
    std::pair<uint16_t, uint16_t> result = {0, 0};
    auto ite = km_to_game_mapping.find(from);
    uint16_t old_to_id = 0;
    if (ite != km_to_game_mapping.end()) {
        if (ite->second == to_id) {
            return result;
        }
        old_to_id = ite->second;
        game_to_km_mapping.erase(old_to_id);
    }
    auto ite2 = game_to_km_mapping.find(to_id);
    if (ite2 != game_to_km_mapping.end()) {
        if (old_to_id == 0) {
            km_to_game_mapping.erase(ite2->second);
        } else {
            /* swap old key and new mapped key */
            km_to_game_mapping[ite2->second] = old_to_id;
            game_to_km_mapping[old_to_id] = ite2->second;
            result = std::make_pair(ite2->second, old_to_id);
        }
    }
    km_to_game_mapping[from] = to_id;
    game_to_km_mapping[to_id] = from;
    return result;
}

void input_base::assign_port(uint32_t device_id, uint8_t port) {
    if (port >= ports.size() || ports[port].device_id == device_id) {
        return;
    }
    port_mapping.erase(ports[port].device_id);
    port_mapping[device_id] = port;
    ports[port].enabled = true;
    ports[port].device_id = device_id;
}

void input_base::unassign_port(uint8_t port) {
    if (port >= ports.size()) {
        return;
    }
    ports[port].enabled = false;
    ports[port].device_id = 0xFFFFFFFFu;
}

void input_base::on_km_input(uint16_t id, bool pressed) {
    if (mode == mode_input) {
        last_input = id;
        return;
    }

    auto ite = port_mapping.find(0);
    if (ite == port_mapping.end()) {
        return;
    }

    auto &p = ports[ite->second];
    auto ite2 = km_to_game_mapping.find(id);
    if (ite2 == km_to_game_mapping.end()) return;
    auto index = ite2->second >> 8;
    auto btn_id = ite2->second & 0xFFu;
    switch (index) {
    case 0:
        if (pressed) {
            p.states |= 1 << btn_id;
        } else {
            p.states &= ~(1 << btn_id);
        }
        break;
    case 1:
    case 2:
        if (btn_id < 2) {
            p.analog_axis[((index - 1) << 1) + btn_id] = pressed ? 0x7FFF : -0x8000;
        }
        break;
    default:
        break;
    }
}

void input_base::on_btn_input(uint32_t device_id, uint8_t id, bool pressed) {
    auto ite = port_mapping.find(device_id);
    if (ite == port_mapping.end()) {
        return;
    }
    auto &p = ports[ite->second];
    if (pressed) {
        p.states |= 1 << id;
    } else {
        p.states &= ~(1 << id);
    }
}

void input_base::on_axis_input(uint32_t device_id, uint8_t id, int16_t value) {
    if (id >= 4) {
        return;
    }
    auto ite = port_mapping.find(device_id);
    if (ite == port_mapping.end()) {
        return;
    }
    auto &p = ports[ite->second];
    p.analog_axis[id] = value;
}

bool input_base::on_device_connected(uint32_t device_id, const gamecontrollerdb::GUID &guid) {
    const auto *controller = gcdb.matchController(guid);
    if (!controller) {
        return false;
    }
    gcontrollers[device_id] = controller;
    return true;
}

void input_base::on_device_disconnected(uint32_t device_id) {
    gcontrollers.erase(device_id);
}

void input_base::on_joybtn_input(uint32_t device_id, uint8_t id, bool pressed) {
    auto ite = gcontrollers.find(device_id);
    if (ite == gcontrollers.end()) {
        return;
    }

    auto m = ite->second->getButton(id, pressed);
}

void input_base::on_joyhat_input(uint32_t device_id, uint8_t id, uint8_t value) {
    auto ite = gcontrollers.find(device_id);
    if (ite == gcontrollers.end()) {
        return;
    }
}

void input_base::on_joyaxis_input(uint32_t device_id, uint8_t id, int16_t value) {
    auto ite = gcontrollers.find(device_id);
    if (ite == gcontrollers.end()) {
        return;
    }
}

}
