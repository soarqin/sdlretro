#pragma once

#include <vector>
#include <map>
#include <string>

extern "C" {
struct retro_core_option_definition;
struct retro_core_options_intl;
struct retro_variable;
}

namespace libretro {

/* variable struct */
struct retro_variable_t {
    /* variable name */
    std::string name;
    /* selected index */
    size_t curr_index;
    /* default index */
    size_t default_index;
    /* variable display text */
    std::string label;
    /* variable description */
    std::string info;
    /* visible */
    bool visible;
    /* options list, in pair (display, description) */
    std::vector<std::pair<std::string, std::string>> options;
};

class retro_variables {
public:
    inline void reset() {
        variables_map.clear();
        variables.clear();
        variables_updated = false;
    }
    inline const retro_variable_t *get_variable(const std::string &key) const {
        const auto ite = variables_map.find(key);
        if (ite == variables_map.end()) return nullptr;
        return ite->second;
    }
    inline void set_variable(const std::string &key, size_t index) {
        auto ite = variables_map.find(key);
        if (ite == variables_map.end()) return;
        if (index != ite->second->curr_index && index < ite->second->options.size()) {
            ite->second->curr_index = index;
            variables_updated = true;
        }
    }
    inline void set_variable_visible(const std::string &key, bool visible) {
        auto ite = variables_map.find(key);
        if (ite == variables_map.end()) return;
        ite->second->visible = visible;
    }
    void load_variables_from_cfg(const std::string &filename);
    void save_variables_to_cfg(const std::string &filename);

    void load_variables(const retro_core_option_definition *def);
    void load_variables(const retro_core_options_intl *def);
    void load_variables(const retro_variable *vars);

    void reset_variables();

    inline const std::vector<retro_variable_t> &get_variables() { return variables; }
    inline bool get_variables_updated() const { return variables_updated; }
    inline void set_variables_updated(bool u) { variables_updated = u; }

private:
    std::vector<retro_variable_t> variables;
    std::map<std::string, retro_variable_t*> variables_map;
    bool variables_updated = false;
};

}
