#pragma once

#include <unordered_map>
#include <array>
#include <string>
#include <cstdint>

namespace gamecontrollerdb {

enum Platform {
    PlatformInvalid = -1,
    PlatformAny = 0,
    PlatformWindows,
    PlatformMacOS,
    PlatformLinux,
    PlatformiOS,
    PlatformAndroid,
    PlatformEmscripten,
    PlatformMax,
};

enum Button {
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
    ButtonInput = 0,
    HatInput,
    AxisInput,
};

using GUID = std::array<uint8_t, 16>;

struct HashGUID {
    size_t operator()(const gamecontrollerdb::GUID&) const noexcept;
};

struct InputResult {
    /* Button or Axis id, check enum ButtonOrAxis */
    int id;
    /* Button: 1 for pressed, 0 for unpressed
     * Axis:   value (-0x8000~0x7FFF) */
    int16_t value;
};

class Controller {
    friend class DB;

    struct MappedInput {
        /* check enum ButtonOrAxis
         * id[0]: positive mapped id
         * id[1]: negative mapped id
         * */
        int id[2];

        /* for inputType of ControllerButtonOrAxis:
         *   ButtonInput: unused
         *   HatInput: unused
         *   AxisInput:
         *      0 - process -max to +max
         *     -1 - process -max to 0
         *      1 - process 0 to +max
         */
        int direction;
    };

public:
    InputResult getButton(int id, bool pressed) const;
    InputResult getAxis(int id, int16_t value) const;

private:
    bool processToken(int index, const std::string &token);

private:
    GUID guid;
    std::string name;
    int platform = PlatformAny;

    std::unordered_map<int, MappedInput> btnMapping, axisMapping;
    std::unordered_map<int, std::array<MappedInput, 4>> hatMapping;
};

class DB {
    using ControllersByPlatform = std::array<Controller, PlatformMax>;
public:

    bool addFromFile(const std::string &filename);
    int addFromString(const std::string &content);

    const Controller *matchController(const GUID &guid) const;

private:
    bool addFromLine(const std::string &line);

private:
    std::unordered_map<GUID, ControllersByPlatform, HashGUID> controllers;
};

}
