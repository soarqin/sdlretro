#include "i18n.h"

#include "util.h"
#include "cfg.h"

#include <json.hpp>
#include <xxhash.h>
#include <spdlog/spdlog.h>

#include <cstring>

namespace libretro {

size_t xxh_hasher::operator()(const std::string &s) const noexcept {
    return static_cast<size_t>(XXH64(s.c_str(), s.length(), 0));
}

i18n i18n_obj;

using json = ::nlohmann::json;

static language_info languages[RETRO_LANGUAGE_LAST] = {
    { 0, "en_US", "English (US)" },
    { 1, "ja_JP", "日本語" },
    { 2, "fr-FR", "Français" },
    { 3, "es-ES", "Español" },
    { 4, "de-DE", "Deutsch" },
    { 5, "it-IT", "Italiano" },
    { 6, "nl-NL", "Nederlands" },
    { 7, "pt-BR", "Português (Brasil)" },
    { 8, "pt-PT", "Português (Portugal)" },
    { 9, "ru-RU", "русский язык" },
    { 10, "ko-KR", "한국어" },
    { 11, "zh-TW", "中文 (繁體)" },
    { 12, "zh-CN", "中文 (简体)" },
    { 13, "eo", "Esperanto" },
    { 14, "pl-PL", "Język polski" },
    { 15, "vi-VN", "Tiếng Việt" },
    { 16, "ar", "العَرَبِيَّة" },
    { 17, "el-GR", "Νέα Ελληνικά" },
    { 18, "tr-TR", "Türkçe" },
    { 19, "sk-SK", "slovenčina" },
    { 20, "fa-IR", "فارسی" },
    { 21, "he-IL", "עברית" },
    { 22, "ast-ES", "Asturianu" }
};

void i18n::get_language_list(std::vector<language_info> &info_list) {
    for (auto &li: languages) {
        if (li.id == 0) {
            info_list.push_back(li);
            continue;
        }
        auto filename = g_cfg.get_data_dir() + "/lang/" + li.code + ".json";
        if (util::file_exists(filename)) info_list.push_back(li);
    }
}

bool i18n::load_language_file(int lang) {
    if (lang >= RETRO_LANGUAGE_LAST) return false;

    json j;
    auto filename = g_cfg.get_data_dir() + "/lang/" + languages[lang].code + ".json";
    if (!util::file_exists(filename)) return false;
    try {
        std::string content;
        if (!util::read_file(filename, content)) {
            throw std::bad_exception();
        }
        j = json::parse(content);
    } catch(...) {
        spdlog::error("failed to read input config from {}", filename);
        return false;
    }
    auto &l = localized_text[lang];
    for (auto &p: j.items()) {
        if (p.value().is_string())
            l[p.key()] = p.value().get<std::string>();
    }
    return true;
}

bool i18n::set_language(int lang) {
    if (lang == 0) {
        current = nullptr;
        g_cfg.set_language(lang);
        return true;
    }
    auto ite = localized_text.find(lang);
    if (ite == localized_text.end()) {
        if (!load_language_file(lang)) return false;
        current = &localized_text[lang];
    } else {
        current = &ite->second;
    }
    g_cfg.set_language(lang);
    return true;
}

const char *i18n::get_text(const char *t) {
    if (current == nullptr) return t;
    auto ite = current->find(t);
    if (ite == current->end()) return t;
    return ite->second.c_str();
}

}
