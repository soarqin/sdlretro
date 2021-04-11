#pragma once

#include "gui_base.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <functional>
#include <cstdint>

namespace gui {

enum menu_type {
    menu_static = 0,
    menu_boolean,
    menu_values,
    menu_input,
    menu_path,
};

struct menu_item {
    /* check enum `menu_type` */
    menu_type type;
    /* display text */
    std::string text;
    /* description displayed aside */
    std::string description;
    /* for menu_boolean, it's 0 or 1
     * for menu_values, it's current selected index in member `values`
     * for menu_input, it's input code for certain input implementation
     * ignored for menu_static and menu_path
     */
    uint64_t selected;
    /* used for menu_values */
    std::vector<std::string> values;

    /* for menu_static:
     *   if this is null, event_loop() will return true if button A is pressed
     *   if this is not null, this function will be called on button A pressed
     *   return true to close all levels of menu
     * for menu_boolean and menu_values:
     *   called if selected index is changed
     *   return value is ignored
     */
    std::function<bool(const menu_item&)> callback;

    /* assigned data to alter, ignored for menu_static
     * if callback is not null, this pointer is ignored
     */
    void *data;

    /* for menu_path:  store selected path
     * for menu_input: store input key/button name
     */
    std::string str;
};

class menu_base: public gui_base<menu_base> {
public:
    menu_base(const std::shared_ptr<drivers::driver_base> &d, menu_base *p, std::function<void(menu_base&)> init_func);
    virtual ~menu_base() = default;

    inline void set_title(const std::string &text) { title = text; }
    inline void set_items(std::vector<menu_item> it) { items = std::move(it); }

    inline void set_rect(int x, int y, int w, int h) { menu_x = x; menu_y = y; menu_width = w; menu_height = h; }
    inline void set_line_spacing(int s) { line_spacing = s; }
    inline void set_item_width(int w) { item_width = w; }

    void event_loop() override;

    /* regular operations */
    void move_up();
    void move_down();
    void page_up();
    void page_down();
    void move_first();
    void move_last();
    void value_dec();
    void value_inc();

    /* force refresh all items of menu */
    inline void force_refresh(bool recursive = true) {
        force_refreshing = true;
        if (recursive && parent != nullptr) parent->force_refresh();
    }

    inline bool get_ok_pressed() const { return ok_pressed; }
    inline size_t get_selected() const { return selected; }
    void set_selected(size_t sel);
    inline void set_init_sel(size_t sel) { init_sel = sel; }
    inline std::vector<menu_item> &get_items() { return items; }

protected:
    /* poll all inputs, return true if any changes to the menu (aka redraw is required) */
    bool poll_input();

    inline void set_ok_pressed(bool b) { ok_pressed = b; }
    /* return maximum count for items for a page */
    virtual size_t page_count() = 0;

protected:
    size_t top_index = 0;
    size_t selected = 0;
    size_t init_sel = 0;
    std::string title;
    std::vector<menu_item> items;
    int menu_x = 0, menu_y = 0;
    int menu_width = 0, menu_height = 0;
    int line_spacing = 4;
    int item_width = 0;
    bool in_input_mode = false;

private:
    bool ok_pressed = false;
    bool force_refreshing = false;

    std::function<void(menu_base&)> init_fn;
};

}
