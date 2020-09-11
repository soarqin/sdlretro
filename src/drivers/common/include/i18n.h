#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <string>

namespace drivers {

struct language_info {
    int id;
    const char *code;
    const char *name;
};

struct xxh_hasher {
    size_t operator()(const std::string &s) const noexcept;
};

class i18n {
public:
    static void get_language_list(std::vector<language_info> &info_list);

    bool load_language_file(int lang);

    bool set_language(int lang);

    const char *get_text(const char *t);

private:
    /* localized text vectors indexed by language */
    std::map<int, std::unordered_map<std::string, std::string, xxh_hasher>> localized_text;

    /* current localized text vector */
    std::unordered_map<std::string, std::string, xxh_hasher> *current = nullptr;
};

extern i18n i18n_obj;

#define _I18N(t) ::drivers::i18n_obj.get_text(t)

}

inline const char *operator "" _i18n(const char *t, size_t) {
    return ::drivers::i18n_obj.get_text(t);
}
