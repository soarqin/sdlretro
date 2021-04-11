#pragma once

#include "gui_base.h"

#include <array>
#include <cstdint>

namespace gui {

template<class T>
class elem_base: public gui_base<T> {
public:
    explicit elem_base(const std::shared_ptr<drivers::driver_base>& d, T *p, int z, int x, int y, int w, int h): gui_base<T>(d, p, z), pos_x(x), pos_y(y), width(w), height(h) {}
    ~elem_base() override = default;

protected:
    int pos_x, pos_y, width, height;
};

class top_window: public elem_base<top_window> {
public:
    using elem_base::elem_base;

    void draw() override;

protected:
    void init() override;
    void deinit() override;
};

class progressbar_base: public elem_base<progressbar_base> {
public:
    using elem_base::elem_base;

    inline void set_progress(uint32_t prog) {
        progress = prog > prog_max ? prog_max : prog;
    }
    inline uint32_t get_progress() const { return progress; }

    inline void set_progress_max(uint32_t progmax) {
        prog_max = progmax;
        if (progress > progmax) progress = progmax;
    }
    inline uint32_t get_progress_max() const { return prog_max; }

    inline void set_box_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        box_color = {r, g, b, a};
    }
    inline void set_bar_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        bar_color = {r, g, b, a};
    }

protected:
    uint32_t progress = 0, prog_max = 100;
    std::array<uint8_t, 4> box_color = {0xFF, 0xFF, 0xFF, 0xFF};
    std::array<uint8_t, 4> bar_color = {0xFF, 0xFF, 0xFF, 0xFF};
};

}
