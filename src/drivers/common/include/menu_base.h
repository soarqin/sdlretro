#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace drivers {

class driver_base;

class menu_base {
protected:
    struct menu_item {
        std::string text;
        std::string description;
        std::vector<std::string> values;
        size_t selected;
    };

public:
    inline menu_base(driver_base *d, bool t): driver(d), topmenu(t) {}
    virtual ~menu_base() = default;

    inline void set_position(int x, int y) { pos_x = x; pos_y = y; }

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

    inline size_t get_selected() { return selected; }

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
    driver_base *driver = nullptr;
    bool topmenu = false;
    size_t top_index = 0;
    size_t selected = 0;
    std::vector<menu_item> items;
    int pos_x = 0, pos_y = 0;

private:
    bool running = false;
    bool ok_pressed = false;
};

}
