#include "gamecontrollerdb.h"

#include <fstream>
#include <cstdlib>

namespace gamecontrollerdb {

size_t HashGUID::operator()(const GUID &guid) const noexcept {
    std::hash<uint64_t> h;
    return h(*reinterpret_cast<const uint64_t*>(guid.data()))
           ^ h(*reinterpret_cast<const uint64_t*>(guid.data() + 8));
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

inline int convertControllerButtonOrAxisToInt(const ControllerButtonOrAxis &cboa) {
    return static_cast<int>(cboa.inputType)
           | (static_cast<int>(cboa.id) << 8)
           | (static_cast<int>(cboa.value) << 16);
}

int Controller::inputMapping(const ControllerButtonOrAxis &cboa) const {
    auto ite = buttonOrAxisMapping.find(convertControllerButtonOrAxisToInt(cboa));
    if (ite == buttonOrAxisMapping.end()) {
        return -1;
    }
    return ite->second;
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
    auto ite = _buttonNameMap.find(key);
    if (ite != _buttonNameMap.end()) {
        ControllerButtonOrAxis cboa {};
        switch (value[0]) {
        case '+':
        case '-':
            if (value.length() < 3 || value[1] != 'a') {
                return false;
            }
            cboa.inputType = AxisInput;
            cboa.id = std::strtol(value.c_str() + 2, nullptr, 10);
            cboa.value = value[0] == '+' ? 1 : -1;
            break;
        case 'a':
            if (value.length() < 2) {
                return false;
            }
            cboa.inputType = AxisInput;
            cboa.id = std::strtol(value.c_str() + 1, nullptr, 10);
            cboa.value = 0;
            break;
        case 'b':
            if (value.length() < 2) {
                return false;
            }
            cboa.inputType = ButtonInput;
            cboa.id = std::strtol(value.c_str() + 1, nullptr, 10);
            cboa.value = 0;
            break;
        case 'h': {
            if (value.length() < 4) {
                return false;
            }
            cboa.inputType = HatInput;
            char *endptr;
            cboa.id = std::strtol(value.c_str() + 1, &endptr, 10);
            if (endptr == nullptr || *endptr != '.') {
                return false;
            }
            cboa.value = std::strtol(endptr + 1, nullptr, 10);;
            break;
        }
        }
        buttonOrAxisMapping[convertControllerButtonOrAxisToInt(cboa)] = ite->second;
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

}
