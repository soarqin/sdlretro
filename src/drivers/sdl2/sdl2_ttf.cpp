#include "sdl2_ttf.h"

#include "helper.h"

#include <glad/glad.h>
#include <SDL.h>

namespace drivers {

sdl2_ttf::~sdl2_ttf() {
    for (auto t: textures) {
        glDeleteTextures(1, &t);
    }
    textures.clear();
}

uint8_t *sdl2_ttf::prepare_texture(size_t index, uint16_t x, uint16_t y, uint16_t w, uint16_t h, int &pitch) {
    static uint8_t sdata[64 * 64];
    pitch = w;
    return sdata;
}

void sdl2_ttf::finish_texture(uint8_t *data, size_t index, uint16_t x, uint16_t y, uint16_t w, uint16_t h, int pitch) {
    if (index >= textures.size()) {
        textures.resize(index + 1, 0u);
    }
    uint32_t &tex = textures[index];
    if (tex == 0) {
        auto rpw = get_rect_pack_width();
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, rpw, rpw, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, pitch, h, GL_RED, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void sdl2_ttf::set_draw_color(uint8_t r, uint8_t g, uint8_t b) {
    color[0] = (float)r / 255.f;
    color[1] = (float)g / 255.f;
    color[2] = (float)b / 255.f;
}

void sdl2_ttf::render(int x, int y, const char *text, int width, int height, bool shadow) {
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
    glUseProgram(program_font);
    glBindVertexArray(vao_font);
    while (*text != 0) {
        uint32_t ch = helper::utf8_to_ucs4(text);
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
                shx0, shy1, sx0, sy1, // bottom left
                shx1, shy1, sx1, sy1  // bottom right
            };
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            glEnableVertexAttribArray(1);
            glUniform3f(uniform_font_color, 0.f, 0.f, 0.f);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
        float vertices[] = {
            x0, y0, sx0, sy0, // top left
            x1, y0, sx1, sy0, // top right
            x0, y1, sx0, sy1, // bottom left
            x1, y1, sx1, sy1  // bottom right
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glUniform3fv(uniform_font_color, 1, color);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        x += fd->advW;
        nwidth -= fd->advW;
    }
}

}
