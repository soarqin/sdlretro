#pragma once

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
public:
    ttf_font();
    virtual ~ttf_font();
    void init(int size, uint8_t width = 0);
    bool add(const std::string& filename, int index = 0);
    uint8_t get_char_width(uint16_t ch) const;

private:
    void new_rect_pack();

protected:
    const font_data *make_cache(uint16_t);
    const uint8_t *get_rect_pack_data(uint8_t idx, int16_t x, int16_t y);
    static uint16_t get_rect_pack_width();

protected:
    std::string ttf_filename;
    int font_size = 16;
    std::vector<font_info> fonts;
    std::unordered_map<uint16_t, font_data> font_cache;
    uint8_t mono_width = 0;

private:
    std::vector<rect_pack_data*> rectpack_data;
#ifndef USE_STB_TRUETYPE
    FT_Library ft_lib = {};
#endif
};

}
