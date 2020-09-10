#pragma once

#include <map>
#include <unordered_map>
#include <string>

namespace drivers {

class i18n {
public:
    void load_language_file(int lang, const std::string &filename);

    inline void set_language(int lang) { language = lang; }
    inline int get_language() const { return language; }

    const char *get_text(const char *);

private:
    /* check enum retro_language in libretro.h */
    int language = 0;

    /* localized text vectors indexed by language */
    std::map<int, std::unordered_map<const char*, std::string>> localized_text;

    /* current localized text vector */
    std::unordered_map<const char*, std::string> *current = nullptr;
};

extern i18n i18n_obj;
#define _I18N(t) ::drivers::i18n_obj.get_text(t)

}
