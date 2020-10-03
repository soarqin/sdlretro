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
    AxisFirst = ButtonMax,
    AxisLeftX = AxisFirst,
    AxisLeftY,
    AxisRightX,
    AxisRightY,
    AxisMax,
};

enum {
    AxisCount = AxisMax - AxisFirst,
};

using GUID = std::array<uint8_t, 16>;

class Controller;

class ControllerState {
public:
    inline explicit ControllerState(const Controller *c, uint16_t *stateVal = nullptr, std::array<int16_t, AxisCount> *axisStateVal = nullptr):
                                    controller(c), state(stateVal), axisState(axisStateVal) {}
    inline void assign(uint16_t *stateVal, std::array<int16_t, AxisCount> *axisStateVal) {
        state = stateVal;
        axisState = axisStateVal;
    }
    void reset();
    void buttonInput(int id, bool pressed);
    void axisInput(int id, int16_t value);
    void hatInput(int id, uint32_t value);

private:
    const Controller *controller;

    uint16_t *state;
    std::array<int16_t, AxisCount> *axisState;
};

class ControllerStateManaged: public ControllerState {
public:
    inline explicit ControllerStateManaged(const Controller *c):
                    ControllerState(c, &stateVal, &axisStateVal) {}
    inline uint16_t getState() const { return stateVal; }
    inline int16_t getAxisState(int id) const {
        if (id < AxisFirst || id >= AxisMax) {
            return 0;
        }
        return axisStateVal[id - AxisFirst];
    }

private:
    uint16_t stateVal = 0;
    std::array<int16_t, AxisCount> axisStateVal = {};
};

class Controller {
    friend class DB;

public:
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

    const GUID &getGUID() const { return guid; }
    const std::string &getName() const { return name; }
    int getPlatform() const { return platform; }
    const MappedInput *btnMap(int id) const;
    const MappedInput *axisMap(int id) const;
    const std::array<MappedInput, 4> *hatMap(int id) const;

private:
    bool processToken(int index, const std::string &token);

private:
    GUID guid;
    std::string name;
    int platform = PlatformAny;

    std::unordered_map<int, MappedInput> btnMapping, axisMapping;
    std::unordered_map<int, std::array<MappedInput, 4>> hatMapping;
};

struct HashGUID {
    size_t operator()(const gamecontrollerdb::GUID&) const noexcept;
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
