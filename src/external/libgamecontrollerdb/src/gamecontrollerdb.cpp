#include "gamecontrollerdb.h"

#include <map>
#include <fstream>
#include <cstdlib>
#include <cstring>

namespace gamecontrollerdb {

enum {
    AxisThreshold = 0x2000,
};

void ControllerState::reset() {
    state = 0;
    std::memset(axisState, 0, sizeof(axisState));
}

void ControllerState::buttonInput(int id, bool pressed) {
    const auto *mi = controller->btnMap(id);
    if (mi == nullptr || mi->id[0] < 0 || mi->id[0] >= AxisMax) {
        return;
    }
    switch (mi->id[0]) {
    case AxisLeftX:
    case AxisLeftY:
    case AxisRightX:
    case AxisRightY:
        if (pressed) {
            if (mi->direction >= 0) {
                axisState[mi->id[0] - AxisFirst] = 0x7FFF;
            } else {
                axisState[mi->id[0] - AxisFirst] = -0x8000;
            }
        } else {
            axisState[mi->id[0] - AxisFirst] = 0;
        }
        break;
    default:
        if (pressed) {
            state |= 1u << mi->id[0];
        } else {
            state &= ~(1u << mi->id[0]);
        }
        break;
    }
}

void ControllerState::axisInput(int id, int16_t value) {
    const auto *mi = controller->axisMap(id);
    if (mi == nullptr) {
        return;
    }
    int direction = value >= AxisThreshold ? 1 : (value <= -AxisThreshold ? -1 : 0);
    if (direction >= 0 && mi->id[0] >= 0 & mi->id[0] <= AxisMax) {
        switch (mi->id[0]) {
        case AxisLeftX:
        case AxisLeftY:
        case AxisRightX:
        case AxisRightY:
            axisState[mi->id[0] - AxisFirst] = mi->direction >= 0 ? value : -value;
            break;
        default:
            if (direction > 0) {
                state |= 1u << mi->id[0];
            } else {
                state &= ~(1u << mi->id[0]);
            }
            break;
        }
    }
    if (direction <= 0 && mi->id[0] != mi->id[1] && mi->id[1] >= 0 && mi->id[1] <= AxisMax) {
        switch (mi->id[1]) {
        case AxisLeftX:
        case AxisLeftY:
        case AxisRightX:
        case AxisRightY:
            axisState[mi->id[1] - AxisFirst] = mi->direction <= 0 ? value : -value;
            break;
        default:
            if (mi->id[1] == mi->id[0]) {
                break;
            }
            if (direction > 0) {
                state |= 1u << mi->id[1];
            } else {
                state &= ~(1u << mi->id[1]);
            }
            break;
        }
    }
}

void ControllerState::hatInput(int id, uint32_t value) {
    const auto *mia = controller->hatMap(id);
    if (mia == nullptr) {
        return;
    }
    const uint32_t flag[4] = {1, 2, 4, 8};
    for (int i = 0; i < 4; ++i) {
        auto &mi = (*mia)[i];
        auto inputId = mi.id[0];
        if (inputId < 0 || inputId >= AxisMax) {
            continue;
        }
        switch (inputId) {
        case AxisLeftX:
        case AxisLeftY:
        case AxisRightX:
        case AxisRightY:
            if (value & flag[i]) {
                if (mi.direction >= 0) {
                    axisState[inputId - AxisFirst] = 0x7FFF;
                } else {
                    axisState[inputId - AxisFirst] = -0x8000;
                }
            } else {
                axisState[inputId - AxisFirst] = 0;
            }
            break;
        default:
            if (value & flag[i]) {
                state |= 1u << inputId;
            } else {
                state &= ~(1u << inputId);
            }
            break;
        }
    }
}

const Controller::MappedInput *Controller::btnMap(int id) const {
    const auto ite = btnMapping.find(id);
    if (ite == btnMapping.end()) {
        return nullptr;
    }
    return &ite->second;
}

const Controller::MappedInput *Controller::axisMap(int id) const {
    const auto ite = axisMapping.find(id);
    if (ite == axisMapping.end()) {
        return nullptr;
    }
    return &ite->second;
}

const std::array<Controller::MappedInput, 4> *Controller::hatMap(int id) const {
    const auto ite = hatMapping.find(id);
    if (ite == hatMapping.end()) {
        return nullptr;
    }
    return &ite->second;
}

inline bool convertGUID(GUID &guid, const std::string &guidStr) {
    size_t len = guidStr.length() & ~1u;
    if (len > guid.size() * 2) {
        len = guid.size() * 2;
    }
    for (size_t i = 0; i < len; i += 2) {
        char hex[3] = {guidStr[i], guidStr[i + 1], 0};
        char *end_ptr;
        guid[i >> 1u] = static_cast<uint8_t>(std::strtoul(hex, &end_ptr, 16));
        if (end_ptr && end_ptr - hex < 2) {
            /* not a valid hex string */
            return false;
        }
    }
    return true;
}

bool Controller::processToken(int index, const std::string &token) {
    static const std::unordered_map<std::string, int> _buttonNameMap = {
        { "a", ButtonA },
        { "b", ButtonB },
        { "x", ButtonX },
        { "y", ButtonY },
        { "back", ButtonBack },
        { "guide", ButtonGuide },
        { "start", ButtonStart },
        { "leftstick", ButtonLeftstick },
        { "rightstick", ButtonRightstick },
        { "leftshoulder", ButtonLeftshoulder },
        { "rightshoulder", ButtonRightshoulder },
        { "dpup", ButtonDpadUp },
        { "dpdown", ButtonDpadDown },
        { "dpleft", ButtonDpadLeft },
        { "dpright", ButtonDpadRight },
        { "leftx", AxisLeftX },
        { "lefty", AxisLeftY },
        { "rightx", AxisRightX },
        { "righty", AxisRightY },
    };

    static const std::unordered_map<std::string, size_t> _platformNameMap = {
        { "Windows", PlatformWindows },
        { "Mac OS X", PlatformMacOS },
        { "Linux", PlatformLinux },
        { "iOS", PlatformiOS },
        { "Android", PlatformAndroid },
        { "Emscripten", PlatformEmscripten }
    };

    if (index == 0) {
        if (!convertGUID(guid, token)) {
            return false;
        }
        return true;
    }
    if (index == 1) {
        name = token;
        return true;
    }

    auto pos = token.find(':');
    if (pos == std::string::npos) {
        return false;
    }
    auto key = token.substr(0, pos);
    auto value = token.substr(pos + 1);
    if (value.empty()) {
        return false;
    }
    int keyDirection = 0;
    if (key[0] == '+' || key[0] == '-') {
        keyDirection = key[0] == '+' ? 1 : -1;
        key.erase(0, 1);
    }
    auto ite = _buttonNameMap.find(key);
    if (ite != _buttonNameMap.end()) {
        if (keyDirection != 0 && (ite->second < ButtonMax || ite->second >= AxisMax)) {
            return false;
        }
        switch (value[0]) {
        case '+':
        case '-': {
            if (value.length() < 3 || value[1] != 'a') {
                return false;
            }
            auto &a = axisMapping[std::strtol(value.c_str() + 2, nullptr, 10)];
            a = MappedInput {{-1, -1}, keyDirection};
            a.id[value[0] == '+' ? 0 : 1] = static_cast<uint8_t>(ite->second);
            break;
        }
        case 'a':
            if (value.length() < 2) {
                return false;
            }
            axisMapping[std::strtol(value.c_str() + 1, nullptr, 10)] = MappedInput {{static_cast<uint8_t>(ite->second), static_cast<uint8_t>(ite->second)}, keyDirection};
            break;
        case 'b':
            if (value.length() < 2) {
                return false;
            }
            btnMapping[std::strtol(value.c_str() + 1, nullptr, 10)] = MappedInput {{static_cast<uint8_t>(ite->second), -1}, keyDirection};
            break;
        case 'h': {
            if (value.length() < 4) {
                return false;
            }
            char *endptr;
            auto hatIndex = std::strtol(value.c_str() + 1, &endptr, 10);
            if (endptr == nullptr || *endptr != '.') {
                return false;
            }
            auto hatValue = std::strtol(endptr + 1, nullptr, 10);
            auto &m = hatMapping[hatIndex];
            MappedInput mi {{static_cast<uint8_t>(ite->second), 0}, keyDirection};
            if (hatValue & 1) {
                m[0] = mi;
            }
            if (hatValue & 2) {
                m[1] = mi;
            }
            if (hatValue & 4) {
                m[2] = mi;
            }
            if (hatValue & 8) {
                m[3] = mi;
            }
            break;
        }
        }
        return true;
    }
    if (key == "platform") {
        auto itePlat = _platformNameMap.find(value);
        if (itePlat == _platformNameMap.end()) {
            return false;
        }
        platform = itePlat->second;
    }
    return true;
}


size_t HashGUID::operator()(const GUID &guid) const noexcept {
    std::hash<uint64_t> h;
    return h(*reinterpret_cast<const uint64_t*>(guid.data()))
        ^ h(*reinterpret_cast<const uint64_t*>(guid.data() + 8));
}

bool DB::addFromFile(const std::string &filename) {
    std::ifstream fs(filename);
    if (!fs.is_open()) {
        return false;
    }
    fs.seekg(0, std::ios::end);
    auto size = fs.tellg();
    if (size <= 0) {
        fs.close();
        return false;
    }
    fs.seekg(0, std::ios::beg);
    std::string n(size, 0);
    fs.read(&n[0], size);
    fs.close();
    return addFromString(n) > 0;
}

int DB::addFromString(const std::string &content) {
    int result = 0;
    typename std::string::size_type pos = 0;
    while (pos != std::string::npos) {
        auto end_pos = content.find_first_of("\r\n", pos);
        auto line = content.substr(pos, end_pos == std::string::npos ? std::string::npos : (end_pos - pos));
        pos = content.find_first_not_of("\r\n", end_pos);
        addFromLine(line);
    }
    return result;
}

bool DB::addFromLine(const std::string &line) {
    typename std::string::size_type pos = 0;
    Controller c;
    int index = 0;
    auto len = line.length();
    while (pos != std::string::npos && pos < len) {
        auto start_pos = line.find_first_not_of(" \t\v\r\n", pos);
        if (start_pos == std::string::npos) {
            break;
        }
        auto end_pos = line.find(',', pos);
        std::string token;
        if (end_pos == std::string::npos) {
            token = line.substr(start_pos);
            pos = std::string::npos;
        } else {
            token = line.substr(start_pos, end_pos - start_pos);
            pos = end_pos + 1;
        }
        if (!c.processToken(index, token)) {
            return false;
        }
        ++index;
    }
    controllers[c.guid][c.platform] = std::move(c);
    return true;
}

const Controller *DB::matchController(const GUID &guid) const {
    auto ite = controllers.find(guid);
    if (ite == controllers.end()) {
        return nullptr;
    }

    auto &cp = ite->second;
#if defined(_WIN32)
    if (!cp[PlatformWindows].name.empty()) {
        return &cp[PlatformWindows];
    }
#endif
#if defined(__linux__)
#if defined(__ANDROID__)
    if (!cp[PlatformAndroid].name.empty()) {
        return &cp[PlatformAndroid];
    }
#else
    if (!cp[PlatformLinux].name.empty()) {
        return &cp[PlatformLinux];
    }
#endif
#endif
#if defined(__APPLE__)
#if defined(IPHONE)
    if (!cp[PlatformAny].name.empty()) {
        return &cp[PlatformiOS];
    }
#elif defined(__MACH__)
    if (!cp[PlatformMacOS].name.empty()) {
        return &cp[PlatformMacOS];
    }
#endif
#endif
#if defined(__EMSCRIPTEN__)
    if (!cp[PlatformEmscripten].name.empty()) {
        return &cp[PlatformEmscripten];
    }
#endif
    if (!cp[PlatformAny].name.empty()) {
        return &cp[PlatformAny];
    }
    return nullptr;
}
}
