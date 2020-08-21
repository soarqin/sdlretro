#include "variables.h"

#include "libretro.h"
#include "util.h"

#include <json.hpp>
#include <spdlog/spdlog.h>

namespace libretro {

void retro_variables::load_variables(const retro_core_option_definition *def) {
    variables.clear();
    while (def->key != nullptr) {
        retro_variable_t *var = nullptr;
        for (auto &v: variables) {
            if (v.name == def->key) {
                var = &v;
                break;
            }
        }
        if (var == nullptr) {
            variables.resize(variables.size() + 1);
            var = &variables.back();
            var->name = def->key;
        }
        var->curr_index = 0;
        var->default_index = 0;
        var->label = def->desc ? def->desc : "";
        var->info = def->info ? def->info : "";
        const auto *opt = def->values;
        while (opt->value != nullptr) {
            if (strcmp(opt->value, def->default_value)==0)
                var->curr_index = var->default_index = var->options.size();
            var->options.emplace_back(std::make_pair(opt->value, opt->label == nullptr ? "" : opt->label));
            ++opt;
        }
        var->visible = true;
        ++def;
    }
    for (auto &var: variables)
        variables_map[var.name] = &var;
}

void retro_variables::load_variables(const retro_variable *vars) {
    variables.clear();
    while (vars->key != nullptr) {
        std::string value = vars->value;
        auto pos = value.find("; ");
        if (pos != std::string::npos) {
            retro_variable_t *var = nullptr;
            for (auto &v: variables) {
                if (v.name == vars->key) {
                    var = &v;
                    break;
                }
            }
            if (var == nullptr) {
                variables.resize(variables.size() + 1);
                var = &variables.back();
            }
            var->curr_index = 0;
            var->default_index = 0;
            var->label = value.substr(0, pos);
            var->info.clear();
            var->visible = true;
            pos += 2;
            for (;;) {
                int end_pos = value.find('|', pos);
                if (pos < end_pos)
                    var->options.emplace_back(std::make_pair(value.substr(pos, end_pos - pos), std::string()));
                if (end_pos == std::string::npos) {
                    break;
                }
                pos = end_pos + 1;
            }
        }
        ++vars;
    }
    for (auto &var: variables)
        variables_map[var.name] = &var;
}

void retro_variables::load_variables_from_cfg(const std::string &filename) {
    if (!util::file_exists(filename)) return;
    nlohmann::json j;
    try {
        std::string content;
        if (!util::read_file(filename, content))
            throw std::bad_exception();
        j = nlohmann::json::parse(content);
    } catch(...) {
        spdlog::error("failed to read core config from {}", filename);
        return;
    }
    if (!j.is_object()) return;
    for (auto &p: j.items()) {
        auto ite = variables_map.find(p.key());
        if (ite == variables_map.end()) continue;
        auto *var = ite->second;
        auto value = p.value().get<std::string>();
        for (size_t i = 0; i < var->options.size(); ++i) {
            if (value == var->options[i].first) {
                if (var->curr_index != i) {
                    var->curr_index = i;
                    variables_updated = true;
                }
                break;
            }
        }
    }
}

void retro_variables::save_variables_to_cfg(const std::string &filename) {
    nlohmann::json j;
    for (auto &var: variables) {
        j[var.name] = var.options[var.curr_index].first;
    }
    try {
        auto content = j.dump(4);
        if (!util::write_file(filename, content))
            throw std::bad_exception();
    } catch(...) {
        spdlog::error("failed to write config to {}", filename);
    }
}

}
