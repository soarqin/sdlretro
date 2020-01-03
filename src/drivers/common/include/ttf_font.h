#pragma once

#include "stb_rect_pack.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <array>

#ifndef USE_STB_TRUETYPE
extern "C" {
typedef struct FT_LibraryRec_ *FT_Library;
typedef struct FT_FaceRec_    *FT_Face;
}
#endif

namespace drivers {

struct rect_pack_data;

class ttf_font {
protected:
    enum {
        RECTPACK_WIDTH = 1024
    };
    struct font_data {
        int16_t rpx, rpy;
        uint8_t rpidx;

        int8_t ix0, iy0;
        uint8_t w, h;
        uint8_t advW;
    };
    struct font_info {
#ifdef USE_STB_TRUETYPE
        float font_scale = 0.f;
		void *font = nullptr;
		uint8_t *ttf_buffer = nullptr;
#else
        FT_Face face = nullptr;
#endif
    };
    struct rect_pack_data {
        stbrp_context context;
        stbrp_node nodes[RECTPACK_WIDTH];
        uint8_t pixels[RECTPACK_WIDTH * RECTPACK_WIDTH];
    };
public:
    ttf_font();
    virtual ~ttf_font();
    void init(int size, uint8_t width = 0);
    bool add(const std::string& filename, int index = 0);

private:
    void new_rect_pack();

protected:
    const font_data *make_cache(uint16_t);
    static uint32_t utf8_to_ucs4(const char *&text);

protected:
    std::string ttf_filename;
    int font_size = 16;
    std::vector<font_info> fonts;
    std::unordered_map<uint16_t, font_data> font_cache;
    std::vector<rect_pack_data*> rectpack_data;
    uint8_t mono_width = 0;

private:
#ifndef USE_STB_TRUETYPE
    FT_Library ft_lib;
#endif
};

}
