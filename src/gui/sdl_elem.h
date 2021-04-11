#pragma once

#include "elem_base.h"

namespace gui {

class sdl_progressbar: public progressbar_base {
public:
    void draw() override;
};

}
