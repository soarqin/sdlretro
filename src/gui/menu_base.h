#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <functional>
#include <cstdint>

namespace drivers {
class driver_base;
}

namespace gui {

enum menu_type {
    menu_static = 0,
    menu_boolean,
    menu_values,
    menu_path,
};

struct menu_item {
    /* check enum `menu_type` */
    menu_type type;
    /* display text */
    std::string text;
    /* description displayed aside */
    std::string description;
    /* current selected index
     * for menu_boolean, its 0 or 1
     * for menu_values, its index in member `values`
     * ignored for menu_static and menu_path
     */
    size_t selected;
    /* used for menu_values */
    std::vector<std::string> values;

    /* for menu_static:
     *   if this is null, enter_menu_loop() will return true if button A is pressed
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

    /* use for menu_path, store selected path */
    std::string path;
};

class menu_base {
public:
    inline menu_base(std::shared_ptr<drivers::driver_base> d, bool t): driver(std::move(d)), topmenu(t) {}
    virtual ~menu_base() = default;

    inline void set_title(const std::string &text) { title = text; }
    inline void set_items(std::vector<menu_item> it) { items = std::move(it); }

    inline void set_rect(int x, int y, int w, int h) { menu_x = x; menu_y = y; menu_width = w; menu_height = h; }
    inline void set_line_spacing(int s) { line_spacing = s; }
    inline void set_item_width(int w) { item_width = w; }

    /* enter menu loop
     * return if `OK` is pressed and use get_selected() to fetch index*/
    bool enter_menu_loop();
    /* call this to leave menu loop */
    void leave_menu_loop();
    /* regular operations */
    void move_up();
    void move_down();
    void page_up();
    void page_down();
    void move_first();
    void move_last();
    void value_dec();
    void value_inc();

    inline size_t get_selected() const { return selected; }

protected:
    inline void set_ok_pressed(bool b) { ok_pressed = b; }

protected:
    /* called when entering menu, doing initialization here */
    virtual void enter() = 0;
    /* called when leaving menu, doing deinitialization here */
    virtual void leave() = 0;
    /* draw menu items */
    virtual void draw() = 0;
    /* return maximum count for items for a page */
    virtual size_t page_count() = 0;
    /* poll all inputs, return true if any changes to the menu (aka redraw is required) */
    virtual bool poll_input();

protected:
    std::shared_ptr<drivers::driver_base> driver;
    bool topmenu = false;
    size_t top_index = 0;
    size_t selected = 0;
    std::string title;
    std::vector<menu_item> items;
    int menu_x = 0, menu_y = 0;
    int menu_width = 0, menu_height = 0;
    int line_spacing = 2;
    int item_width = 0;

private:
    bool running = false;
    bool ok_pressed = false;
};

}
