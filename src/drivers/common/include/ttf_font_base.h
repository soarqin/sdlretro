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

class ttf_font_base {
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
		std::vector<uint8_t> ttf_buffer;
#else
        FT_Face face = nullptr;
#endif
    };
public:
    ttf_font_base();
    virtual ~ttf_font_base();
    void init(int size, uint8_t width = 0);
    bool add(const std::string& filename, int index = 0);
    void get_char_width_and_height(uint16_t ch, uint8_t &width, int8_t &t, int8_t &b);

    inline int get_font_size() const { return font_size; }

private:
    void new_rect_pack();

protected:
    inline const font_data *get_cache(uint16_t ch) {
        auto ite = font_cache.find(ch);
        if (ite != font_cache.end()) return &ite->second;
        return make_cache(ch);
    }
    const font_data *make_cache(uint16_t);
    virtual uint8_t *prepare_texture(size_t index, uint16_t x, uint16_t y, uint16_t w, uint16_t h, int &pitch) = 0;
    virtual void finish_texture(uint8_t *data, size_t index, uint16_t x, uint16_t y, uint16_t w, uint16_t h, int pitch) {}
    static uint16_t get_rect_pack_width();

protected:
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
