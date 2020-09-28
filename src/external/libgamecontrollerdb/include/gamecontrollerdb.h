#pragma once

#include <unordered_map>
#include <array>
#include <string>
#include <cstdint>

namespace gamecontrollerdb {

enum {
    PlatformInvalid = -1,
    PlatformAny = 0,
    PlatformWindows,
    PlatformMacOS,
    PlatformLinux,
    PlatformiOS,
    PlatformAndroid,
    PlatformMax,
};

enum {
    ButtonInvalid = -1,
    ButtonA,
    ButtonB,
    ButtonX,
    ButtonY,
    ButtonBack,
    ButtonGuide,
    ButtonStart,
    ButtonLeftstick,
    ButtonRightstick,
    ButtonLeftshoulder,
    ButtonRightshoulder,
    ButtonDpadUp,
    ButtonDpadDown,
    ButtonDpadLeft,
    ButtonDpadRight,
    ButtonMax,
    AxisLeftX = ButtonMax,
    AxisLeftY,
    AxisRightX,
    AxisRightY,
    AxisMax,
};

enum {
    HatUp = 1,
    HatRight = 2,
    HatDown = 4,
    HatLeft = 8,
};

enum {
    ButtonInput = 0,
    HatInput,
    AxisInput,
};

using GUID = std::array<uint8_t, 16>;

struct HashGUID {
    size_t operator()(const gamecontrollerdb::GUID&) const noexcept;
};

struct ControllerButtonOrAxis {
    uint8_t inputType;

    /* ButtonInput: button id
     * HatInput: hat index
     * AxisInput: axis index
     */
    uint8_t id;

    /* ButtonInput: unused
     * HatInput: hat value
     * AxisInput:
     *    0 - process -max to +max
     *   -1 - process -max to 0
     *    1 - process 0 to +max
     */
    int8_t value;
};

class Controller {
    friend class DB;

public:
    int inputMapping(const ControllerButtonOrAxis &cboa) const;

private:
    bool processToken(int index, const std::string &token);

private:
    GUID guid;
    std::string name;
    int platform = PlatformAny;
    std::unordered_map<int, int> buttonOrAxisMapping;
};

class DB {
    using ControllersByPlatform = std::array<Controller, PlatformMax>;
public:

    bool addFromFile(const std::string &filename);
    int addFromString(const std::string &content);

private:
    bool addFromLine(const std::string &line);

private:
    std::unordered_map<GUID, ControllersByPlatform, HashGUID> controllers;
};

}
