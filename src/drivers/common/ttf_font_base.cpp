#include "ttf_font_base.h"

#include "stb_rect_pack.h"

#ifdef USE_STB_TRUETYPE
#include "stb_truetype.h"
#include <helper.h>
#include <cstring>
#else
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

namespace drivers {

enum :uint16_t {
    TTF_RECTPACK_WIDTH = 1024
};

struct rect_pack_data {
    stbrp_context context;
    stbrp_node nodes[TTF_RECTPACK_WIDTH];
};

ttf_font_base::ttf_font_base() {
#ifndef USE_STB_TRUETYPE
    FT_Init_FreeType(&ft_lib);
#endif
}

ttf_font_base::~ttf_font_base() {
    deinit();
#ifndef USE_STB_TRUETYPE
    FT_Done_FreeType(ft_lib);
#endif
}

void ttf_font_base::init(int size, uint8_t width) {
    font_size = size;
    mono_width = width;
}

void ttf_font_base::deinit() {
    font_cache.clear();
    for (auto *&p: rectpack_data) {
        delete p;
    }
    rectpack_data.clear();
    for (auto &p: fonts) {
#ifdef USE_STB_TRUETYPE
        delete static_cast<stbtt_fontinfo *>(p.font);
		p.ttf_buffer.clear();
#else
        FT_Done_Face(p.face);
#endif
    }
    fonts.clear();
}

bool ttf_font_base::add(const std::string &filename, int index) {
    font_info fi;
#ifdef USE_STB_TRUETYPE
    if (!helper::read_file(filename, fi.ttf_buffer)) {
        return false;
    }
    auto *info = new stbtt_fontinfo;
	stbtt_InitFont(info, &fi.ttf_buffer[0], stbtt_GetFontOffsetForIndex(&fi.ttf_buffer[0], index));
	fi.font_scale = stbtt_ScaleForMappingEmToPixels(info, static_cast<float>(font_size));
	fi.font = info;
    fonts.emplace_back(std::move(fi));
#else
    if (FT_New_Face(ft_lib, filename.c_str(), index, &fi.face)) return false;
    FT_Set_Pixel_Sizes(fi.face, 0, font_size);
    fonts.emplace_back(fi);
#endif
    new_rect_pack();
    return true;
}

void ttf_font_base::get_char_width_and_height(uint16_t ch, uint8_t &width, int8_t &t, int8_t &b) {
    const font_data *fd;
    auto ite = font_cache.find(ch);
    if (ite == font_cache.end()) {
        fd = make_cache(ch);
        if (!fd) {
            width = t = b = 0;
            return;
        }
    } else {
        fd = &ite->second;
        if (fd->advW == 0) {
            width = t = b = 0;
            return;
        }
    }
    if (mono_width)
        width = std::max(fd->advW, mono_width);
    else
        width = fd->advW;
    t = fd->iy0;
    b = fd->iy0 + fd->h;
}

const ttf_font_base::font_data *ttf_font_base::make_cache(uint16_t ch) {
    font_info *fi = nullptr;
#ifdef USE_STB_TRUETYPE
    stbtt_fontinfo *info;
    uint32_t index = 0;
#endif
    for (auto &f: fonts) {
        fi = &f;
#ifdef USE_STB_TRUETYPE
        info = static_cast<stbtt_fontinfo*>(f.font);
        index = stbtt_FindGlyphIndex(info, ch);
        if (index == 0) continue;
#else
        auto index = FT_Get_Char_Index(f.face, ch);
        if (index == 0) continue;
        if (!FT_Load_Glyph(f.face, index, FT_LOAD_DEFAULT)) break;
#endif
    }
    font_data *fd = &font_cache[ch];
    if (fi == nullptr) {
        memset(fd, 0, sizeof(font_data));
        return nullptr;
    }

#ifdef USE_STB_TRUETYPE
    /* Read font data to cache */
    int advW, leftB;
    stbtt_GetGlyphHMetrics(info, index, &advW, &leftB);
    fd->advW = static_cast<uint8_t>(fi->font_scale * advW);
    leftB = static_cast<uint8_t>(fi->font_scale * leftB);
    int ix0, iy0, ix1, iy1;
    stbtt_GetGlyphBitmapBoxSubpixel(info, index, fi->font_scale, fi->font_scale, 3, 3, &ix0, &iy0, &ix1, &iy1);
    fd->ix0 = leftB;
    fd->iy0 = iy0;
    fd->w = ix1 - ix0;
    fd->h = iy1 - iy0;
#else
    unsigned char *src_ptr;
    int bitmap_pitch;
    if (FT_Render_Glyph(fi->face->glyph, FT_RENDER_MODE_NORMAL)) return nullptr;
    FT_GlyphSlot slot = fi->face->glyph;
    fd->ix0 = slot->bitmap_left;
    fd->iy0 = -slot->bitmap_top;
    fd->w = slot->bitmap.width;
    fd->h = slot->bitmap.rows;
    fd->advW = slot->advance.x >> 6;
    src_ptr = slot->bitmap.buffer;
    bitmap_pitch = slot->bitmap.pitch;
#endif

    /* Get last rect pack bitmap */
    auto rpidx = rectpack_data.size() - 1;
    auto *rpd = rectpack_data[rpidx];
    stbrp_rect rc = {0, static_cast<uint16_t>((fd->w + 3u) & ~3u), fd->h};
    if (!stbrp_pack_rects(&rpd->context, &rc, 1)) {
        /* No space to hold the bitmap,
         * create a new bitmap */
        new_rect_pack();
        rpidx = rectpack_data.size() - 1;
        rpd = rectpack_data[rpidx];
        stbrp_pack_rects(&rpd->context, &rc, 1);
    }
    /* Do rect pack */
    fd->rpx = rc.x;
    fd->rpy = rc.y;
    fd->rpidx = rpidx;

    int dst_pitch;
    auto *dst = prepare_texture(rpidx, rc.x, rc.y, rc.w, rc.h, dst_pitch);
    auto *dst_ptr = dst;
#ifdef USE_STB_TRUETYPE
    stbtt_MakeGlyphBitmapSubpixel(info, dst, fd->w, fd->h, dst_pitch, fi->font_scale, fi->font_scale, 3, 3, index);
#else
    for (int k = 0; k < fd->h; ++k) {
        memcpy(dst_ptr, src_ptr, fd->w);
        src_ptr += bitmap_pitch;
        dst_ptr += dst_pitch;
    }
#endif
    finish_texture(dst, rpidx, rc.x, rc.y, rc.w, rc.h, dst_pitch);
    return fd;
}

void ttf_font_base::new_rect_pack() {
    auto *rpd = new rect_pack_data;
    stbrp_init_target(&rpd->context, TTF_RECTPACK_WIDTH, TTF_RECTPACK_WIDTH, rpd->nodes, TTF_RECTPACK_WIDTH);
    rectpack_data.push_back(rpd);
}

uint16_t ttf_font_base::get_rect_pack_width() {
    return TTF_RECTPACK_WIDTH;
}

}
