#include "ui_menu.h"

#include "cfg.h"

#include "sdl_menu.h"
#include "i18n.h"
#include "driver_base.h"
#include "input_base.h"
#include "video_base.h"

#include "libretro.h"
#include "variables.h"
#include "core_manager.h"

namespace gui {

std::vector<::libretro::language_info> global_language_list;

ui_menu::ui_menu(std::shared_ptr<drivers::driver_base> drv): driver(std::move(drv)) {
    ::libretro::i18n::get_language_list(global_language_list);
}

int ui_menu::select_core_menu(const std::vector<const libretro::core_info *> &core_list) {
    sdl_menu menu(driver, true);

    menu.set_title("[SELECT CORE TO USE]"_i18n);

    std::vector<menu_item> items;
    for (const auto *core: core_list) {
        menu_item mi = {menu_static, core->name + ' ' + core->version};
        items.emplace_back(mi);
    }
    menu.set_items(items);
    int w, h;
    std::tie(w, h) = g_cfg.get_resolution();
    auto border = w / 16;
    menu.set_rect(border, border, w - border * 2, h - border * 2);
    if (!menu.enter_menu_loop()) return -1;
    return menu.get_selected();
}

void ui_menu::in_game_menu() {
    sdl_menu menu(driver, true);

    menu.set_title("[IN-GAME MENU]"_i18n);

    std::vector<menu_item> items = {
        {menu_static, "Global Settings"_i18n, "", 0, {},
         [this](const menu_item &item) { return global_settings_menu(); }},
        {menu_static, "Core Settings"_i18n, "", 0, {},
         [this](const menu_item &item) { return core_settings_menu(); }},
#if SDLRETRO_FRONTEND > 1
        {menu_static, "Input Settings"_i18n, "", 0, {},
         [this](const menu_item &item) { return input_settings_menu(); }},
#endif
        {menu_static, "Language"_i18n, "", 0, {},
         [this](const menu_item &item) { return language_settings_menu(); }},
        {menu_static, "Reset Game"_i18n, "", 0, {}, [this](const menu_item &) {
            driver->reset();
            return true;
        }},
        {menu_static, "Exit"_i18n, "", 0, {}, [this](const menu_item &) {
            driver->shutdown();
            return true;
        }},
    };
    auto *vari = driver->get_variables();
    const auto &vars = vari->get_variables();
    if (vars.empty()) {
        /* remove `Core Settings` from menu */
        items.erase(items.begin() + 1);
    }
    if (global_language_list.size() < 2) {
        items.erase(items.end() - 3);
    }
    menu.set_items(items);
    int w, h;
    std::tie(w, h) = g_cfg.get_resolution();
    auto border = w / 16;
    menu.set_rect(border, border, w - border * 2, h - border * 2);
    menu.enter_menu_loop();
}

bool ui_menu::global_settings_menu() {
    enum :size_t {
        check_secs_count = 4
    };
    static const uint32_t check_secs[check_secs_count] = {0, 5, 15, 30};

    sdl_menu menu(driver, false);

    menu.set_title("[GLOBAL SETTINGS]"_i18n);

    size_t check_sec_idx;
    const uint32_t *n = std::lower_bound(check_secs, check_secs + check_secs_count, g_cfg.get_save_check());
    check_sec_idx = n - check_secs;
    if (check_sec_idx >= check_secs_count) {
        check_sec_idx = 0;
    }
    std::vector<menu_item> items = {
        {menu_values, "SRAM/RTC Save Interval"_i18n, "", check_sec_idx,
            {"off"_i18n, "5", "15", "30"},
            [](const menu_item &item) -> bool {
                if (item.selected < check_secs_count) {
                    g_cfg.set_save_check(check_secs[item.selected]);
                }
                return false;
            }
        },
#if SDLRETRO_FRONTEND == 2
        {menu_boolean, "Integer Scaling"_i18n, "", static_cast<size_t>(g_cfg.get_integer_scaling() ? 1 : 0),
            {},
            [&](const menu_item &item) -> bool {
                g_cfg.set_integer_scaling(item.selected != 0);
                driver->get_video()->config_changed();
                return false;
            }
        },
        {menu_boolean, "Linear Rendering"_i18n, "", static_cast<size_t>(g_cfg.get_linear() ? 1 : 0),
            {},
            [&](const menu_item &item) -> bool {
                g_cfg.set_linear(item.selected != 0);
                driver->get_video()->config_changed();
                return false;
            }
        },
#endif
    };
    menu.set_items(items);
    int w, h;
    std::tie(w, h) = g_cfg.get_resolution();
    auto border = w / 16;
    menu.set_rect(border, border, w - border * 2, h - border * 2);
    menu.set_item_width(w - border * 2 - 90);
    menu.enter_menu_loop();
    g_cfg.save();
    return false;
}

bool ui_menu::core_settings_menu() {
    auto *vari = driver->get_variables();
    const auto &vars = vari->get_variables();
    if (vars.empty()) return false;
    sdl_menu menu(driver, false);

    menu.set_title("[CORE SETTINGS]"_i18n);

    std::vector<menu_item> items;
    for (auto &var: vars) {
        menu_item item = {menu_values, var.label, var.info, var.curr_index};
        for (auto &opt: var.options)
            item.values.push_back(opt.first);
        item.callback = [this, &vari, &var](const menu_item &item) -> bool {
            vari->set_variable(var.name, item.selected);
            return false;
        };
        items.emplace_back(item);
    }
    menu.set_items(items);
    int w, h;
    std::tie(w, h) = g_cfg.get_resolution();
    auto border = w / 16;
    menu.set_rect(border, border, w - border * 2, h - border * 2);
    menu.set_item_width(w - border * 2 - 90);
    menu.enter_menu_loop();
    driver->save_variables_to_cfg();
    return false;
}

bool ui_menu::input_settings_menu() {
    sdl_menu menu(driver, false);

    menu.set_title("[INPUT SETTINGS]"_i18n);

    std::vector<menu_item> items;
    auto *input = driver->get_input();
    input->foreach_mapping([&](const drivers::output_button_t &o, const drivers::input_button_t &i) {
        menu_item item = {menu_input, o.description, "", i.value};
        item.str = i.name;
        item.data = const_cast<void*>(static_cast<const void*>(&o));
        item.callback = [&menu, &input](const menu_item &item) -> bool {
            const auto *o = static_cast<const drivers::output_button_t*>(item.data);
            input->remove_mapping(o->port, o->id);
            input->add_mapping(item.selected, o->port, o->id);
            for (auto &p: menu.get_items()) {
                if (&p != &item && p.selected == item.selected) {
                    p.selected = 0;
                    std::string device_name;
                    input->get_input_name(p.selected, device_name, p.str);
                    if (!device_name.empty()) {
                        p.str += " (";
                        p.str += device_name;
                        p.str += ')';
                    }
                }
            }
            return false;
        };
        items.emplace_back(item);
    });

    menu.set_items(items);
    int w, h;
    std::tie(w, h) = g_cfg.get_resolution();
    auto border = w / 16;
    menu.set_rect(border, border, w - border * 2, h - border * 2);
    menu.set_item_width(w - border * 2 - 90);
    menu.enter_menu_loop();

    input->save_to_cfg();
    return false;
}

bool ui_menu::language_settings_menu() {
    sdl_menu menu(driver, false);

    menu.set_title("[LANGUAGE]"_i18n);

    std::vector<std::string> lvalues;
    auto lang = g_cfg.get_language();
    size_t idx = 0, lindex = 0;
    std::vector<menu_item> items;

    for (auto &l: global_language_list) {
        menu_item item = {menu_static, l.name, ""};
        item.callback = [&l](const menu_item &item) -> bool {
            ::libretro::i18n_obj.set_language(l.id);
            return true;
        };
        items.emplace_back(item);
        if (l.id == lang) {
            lindex = idx;
        }
        ++idx;
    }
    menu.set_items(items);
    int w, h;
    std::tie(w, h) = g_cfg.get_resolution();
    auto border = w / 16;
    menu.set_rect(border, border, w - border * 2, h - border * 2);
    menu.set_item_width(w - border * 2 - 90);
    menu.enter_menu_loop(lindex);
    return false;
}

}
