#include "i18n.h"

namespace drivers {

i18n i18n_obj;

void i18n::load_language_file(int lang, const std::string &filename) {

}

const char *i18n::get_text(const char *text) {
    if (current == nullptr) return text;
}

}
