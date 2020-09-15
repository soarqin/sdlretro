#include "sdl2_ttf_ogl.h"

#include "util.h"

#include <glad/glad.h>
#include <SDL.h>

namespace drivers {

sdl2_ttf_ogl::~sdl2_ttf_ogl() {
    for (auto t: textures) {
        glDeleteTextures(1, &t);
    }
    textures.clear();
}

const ttf_font_base::font_data *sdl2_ttf_ogl::make_cache(uint16_t ch) {
    const auto *fd = ttf_font_base::make_cache(ch);
    if (fd == nullptr) return fd;
    if (fd->rpidx >= textures.size()) {
        textures.resize(fd->rpidx + 1, 0u);
    }
    uint32_t &tex = textures[fd->rpidx];
    auto w = get_rect_pack_width();
    if (tex == 0) {
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, w, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    auto *src_ptr = get_rect_pack_data(fd->rpidx, fd->rpx, fd->rpy);
    glBindTexture(GL_TEXTURE_2D, tex);
    auto y = fd->rpy;
    for (auto z = fd->h; z; z--) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, fd->rpx, y++, fd->w, 1, GL_RED, GL_UNSIGNED_BYTE, src_ptr);
        src_ptr += w;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    return fd;
}

void sdl2_ttf_ogl::render(int x, int y, const char *text, int width, int height, bool shadow) {
    if (font_size > height)
        return;
    int ox = x;
    int nwidth = width;
    bool allow_wrap = false;
    if (width < 0) {
        allow_wrap = true;
        width = -width;
    }
    auto w = (float)get_rect_pack_width();
    glUseProgram(shader_font);
    glBindVertexArray(vao_font);
    while (*text != 0) {
        uint32_t ch = util::utf8_to_ucs4(text);
        if (ch == 0 || ch > 0xFFFFu) continue;

        const font_data *fd;
        auto ite = font_cache.find(ch);
        if (ite == font_cache.end()) {
            fd = make_cache(ch);
            if (!fd) {
                continue;
            }
        } else {
            fd = &ite->second;
            if (fd->advW == 0) continue;
        }

        int cwidth = mono_width ? std::max(fd->w, mono_width) : fd->w;
        if (cwidth > nwidth) {
            if (!allow_wrap) break;
            x = ox;
            nwidth = width;
            y += font_size + 1;
            height -= font_size;
            if (y > height)
                break;
        }

        auto sx0 = (float)fd->rpx / w;
        auto sy0 = (float)fd->rpy / w;
        auto sx1 = (float)(fd->rpx + fd->w) / w;
        auto sy1 = (float)(fd->rpy + fd->h) / w;
        auto x0 = (float)(x + fd->ix0);
        auto y0 = (float)(y + fd->iy0);
        auto x1 = x0 + (float)fd->w;
        auto y1 = y0 + (float)fd->h;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[fd->rpidx]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_font);
        if (shadow) {
            auto shx0 = x0 + 2.f;
            auto shx1 = x1 + 2.f;
            auto shy0 = y0 + 2.f;
            auto shy1 = y1 + 2.f;
            float vertices[] = {
                shx0, shy0, sx0, sy0, // top left
                shx1, shy0, sx1, sy0, // top right
                shx1, shy1, sx1, sy1, // bottom right
                shx0, shy1, sx0, sy1 // bottom left
            };
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            glEnableVertexAttribArray(1);
            glUniform3f(uniform_font_color, 0.f, 0.f, 0.f);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        }
        float vertices[] = {
            x0, y0, sx0, sy0, // top left
            x1, y0, sx1, sy0, // top right
            x1, y1, sx1, sy1, // bottom right
            x0, y1, sx0, sy1  // bottom left
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glUniform3fv(uniform_font_color, 1, color);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        x += fd->advW;
        nwidth -= fd->advW;
    }
}

void sdl2_ttf_ogl::set_draw_color(uint8_t r, uint8_t g, uint8_t b) {
    color[0] = (float)r / 255.f;
    color[1] = (float)g / 255.f;
    color[2] = (float)b / 255.f;
}

}
