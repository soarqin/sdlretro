#include "sdl2_video_ogl.h"

#include "sdl2_ttf_ogl.h"

#include "cfg.h"

#include "util.h"

#include <spdlog/spdlog.h>

#include <glad/glad.h>

#include <SDL.h>

namespace drivers {

inline uint32_t compile_shader(const char *vertex_shader_source,
                   const char *fragment_shader_source) {
    uint32_t vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);
    // check for shader compile errors
    int success;
    char info_log[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
        spdlog::error("vertex shader compilation failed: {}", info_log);
        glDeleteShader(vertex_shader);
        return 0;
    }
    // fragment shader
    uint32_t fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);
    // check for shader compile errors
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log);
        spdlog::error("fragment shader compilation failed: {}", info_log);
        glDeleteShader(fragment_shader);
        glDeleteShader(vertex_shader);
        return 0;
    }
    // link shaders
    uint32_t shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    // check for linking errors
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, nullptr, info_log);
        spdlog::error("shader program linking failed: {}", info_log);
        glDeleteProgram(shader_program);
        shader_program = 0;
    }
    glDeleteShader(fragment_shader);
    glDeleteShader(vertex_shader);
    return shader_program;
}

sdl2_video_ogl::sdl2_video_ogl() {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    std::tie(curr_width, curr_height) = g_cfg.get_resolution();
    window = SDL_CreateWindow("SDLRetro", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              curr_width, curr_height, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);

    context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, context);

    gladLoadGLLoader(SDL_GL_GetProcAddress);
    glViewport(0, 0, curr_width, curr_height);
    SDL_GL_SetSwapInterval(1);

    init_opengl();

    ttf[0] = std::make_shared<sdl2_ttf_ogl>();
    ttf[0]->init(16, 0);
    ttf[0]->add(g_cfg.get_data_dir() + PATH_SEPARATOR_CHAR + "fonts" + PATH_SEPARATOR_CHAR + "regular.ttf", 0);
    ttf[1] = std::make_shared<sdl2_ttf_ogl>();
    ttf[1]->init(16, 0);
    ttf[1]->add(g_cfg.get_data_dir() + PATH_SEPARATOR_CHAR + "fonts" + PATH_SEPARATOR_CHAR + "bold.ttf", 0);
}

sdl2_video_ogl::~sdl2_video_ogl() {
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
}

bool sdl2_video_ogl::resolution_changed(unsigned width, unsigned height, unsigned pixel_format) {
    game_pixel_format = pixel_format;
    return true;
}

void sdl2_video_ogl::render(const void *data, unsigned width, unsigned height, size_t pitch) {
    if (!data) {
        drawn = false;
        return;
    }
    if (skip_frame) {
        drawn = false;
        skip_frame = false;
        return;
    }
    drawn = true;

    uint32_t pitch_in_pixel;
    switch(game_pixel_format) {
    case 0:
        pitch_in_pixel = pitch >> 1;
        break;
    case 1:
        pitch_in_pixel = pitch >> 2;
        break;
    default:
        pitch_in_pixel = pitch >> 1;
        break;
    }
    if (width != game_width || height != game_height || pitch_in_pixel != game_pixel_format) {
        /*
        if (texture) SDL_DestroyTexture(texture);
         */
        game_width = width;
        game_height = height;
        game_pitch = pitch_in_pixel;
        float ratio;
        if (aspect_ratio <= 0.f) {
            ratio = (float)width / (float)height;
        } else {
            ratio = aspect_ratio;
        }
        float sratio = (float)curr_width / (float)curr_height;
        if (g_cfg.get_integer_scaling()) {
            int int_ratio;
            if (sratio < ratio) {
                int_ratio = (int)std::floor((double)curr_width / width);
            } else {
                int_ratio = (int)std::floor((double)curr_height / height);
            }
            int expected_width = int_ratio * (int)width;
            int expected_height = int_ratio * (int)height;
            display_rect = {((int)curr_width - expected_width) / 2, ((int)curr_height - expected_height) / 2, expected_width, expected_height};
        } else {
            if (sratio < ratio) {
                auto expected_height = (int)std::lround((float)curr_width / ratio);
                display_rect = {0, ((int)curr_height - expected_height) / 2, (int)curr_width, expected_height};
            } else {
                auto expected_width = (int)std::lround(ratio * (float)curr_height);
                display_rect = {((int)curr_width - expected_width) / 2, 0, expected_width, (int)curr_height};
            }
        }
        /*
        switch (game_pixel_format) {
        case 0:
            texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB555, SDL_TEXTUREACCESS_STREAMING, game_pitch, game_height);
            break;
        case 1:
            texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, game_pitch, game_height);
            break;
        default:
            texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, game_pitch, game_height);
            break;
        }
         */
    }
    /*
    void *pixels;
    int lock_pitch;
    if (SDL_LockTexture(texture, nullptr, &pixels, &lock_pitch) == 0) {
        memcpy(pixels, data, pitch * height);
        SDL_UnlockTexture(texture);
    }
     */
    do_render();
    /*
    SDL_RenderPresent(renderer);
     */
}

void *sdl2_video_ogl::get_framebuffer(unsigned int *width, unsigned int *height, size_t *pitch, int *format) {
    return video_base::get_framebuffer(width, height, pitch, format);
}

void sdl2_video_ogl::clear() {
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void sdl2_video_ogl::flip() {
    SDL_GL_SwapWindow(window);
}

int sdl2_video_ogl::get_font_size() const {
    return ttf[0]->get_font_size();
}

void sdl2_video_ogl::set_draw_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    /*
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
     */
}

void sdl2_video_ogl::draw_rectangle(int x, int y, int w, int h) {
    SDL_Rect rc{x, y, w, h};
    /*
    SDL_RenderDrawRect(renderer, &rc);
     */
}

void sdl2_video_ogl::fill_rectangle(int x, int y, int w, int h) {
    SDL_Rect rc{x, y, w, h};
    /*
    SDL_RenderFillRect(renderer, &rc);
     */
}

void sdl2_video_ogl::draw_text(int x, int y, const char *text, int width, bool shadow) {
    if (width == 0) width = (int)curr_width - x;
    else if (width < 0) width = x - (int)curr_width;
    ttf[0]->render(x, y, text, width, (int)curr_height + ttf[0]->get_font_size() - y, shadow);
}

void sdl2_video_ogl::get_text_width_and_height(const char *text, uint32_t &w, int &t, int &b) const {
    w = 0;
    t = 255;
    b = -255;
    while (*text != 0) {
        uint32_t ch = util::utf8_to_ucs4(text);
        if (ch == 0 || ch > 0xFFFFu) continue;
        uint8_t width;
        int8_t tt, tb;
        ttf[0]->get_char_width_and_height(ch, width, tt, tb);
        if (width) {
            w += width;
            if (tt < t) t = tt;
            if (tb > b) b = tb;
        }
    }
}

void sdl2_video_ogl::enter_menu() {
    /*
    if (support_render_to_texture) {
        if (background) {
            SDL_DestroyTexture(background);
        }
        background = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, curr_width, curr_height);
        SDL_SetRenderTarget(renderer, background);
        do_render();

        SDL_BlendMode saved_mode;
        SDL_GetRenderDrawBlendMode(renderer, &saved_mode);
        Uint8 saved_color[4];
        SDL_GetRenderDrawColor(renderer, saved_color, saved_color + 1, saved_color + 2, saved_color + 3);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xA0);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(renderer, nullptr);

        SDL_SetRenderDrawColor(renderer, saved_color[0], saved_color[1], saved_color[2], saved_color[3]);
        SDL_SetRenderDrawBlendMode(renderer, saved_mode);

        SDL_RenderPresent(renderer);
        SDL_SetRenderTarget(renderer, nullptr);
    }
     */
}

void sdl2_video_ogl::leave_menu() {
    /*
    if (background) {
        SDL_DestroyTexture(background);
        background = nullptr;
    }
     */
}

void sdl2_video_ogl::predraw_menu() {
    /*
    SDL_RenderCopy(renderer, background, nullptr, nullptr);
     */
}

void sdl2_video_ogl::config_changed() {
    /*
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, g_cfg.get_linear() ? "1" : "0");
     */
    game_width = game_height = game_pitch = 0;
}

void sdl2_video_ogl::do_render() {
    clear();

    SDL_Rect rc{0, 0, (int)game_width, (int)game_height};
    SDL_Rect target_rc{display_rect[0], display_rect[1], display_rect[2], display_rect[3]};
    /*
    SDL_RenderCopy(renderer, texture, &rc, &target_rc);
     */

    if (messages.empty()) return;
    uint32_t lh = ttf[0]->get_font_size() + 2;
    uint32_t y = curr_height - 5 - (messages.size() - 1) * lh;
    for (auto &m: messages) {
        draw_text(5, y, m.first.c_str(), 0, true);
        y += lh;
    }
}
void sdl2_video_ogl::init_opengl() {
    shader_direct_draw = compile_shader(
        "#version 150 core\n"
        "in vec3 aPos;\n"
        "in vec4 aColor;\n"
        "out vec4 outColor;\n"
        "void main()\n"
        "{\n"
        "  gl_Position = vec4(aPos, 1.0);\n"
        "  outColor = aColor;\n"
        "}",
        "#version 150 core\n"
        "out vec4 fragColor;\n"
        "in vec4 outColor;\n"
        "void main()\n"
        "{\n"
        "  fragColor = outColor;\n"
        "}");
    shader_texture = compile_shader(
        "#version 150 core\n"
        "in vec2 aPos;\n"
        "in vec2 aTexCoord;\n"
        "out vec2 texCoord;\n"
        "void main()\n"
        "{\n"
        "  gl_Position = vec4(aPos, 0.0, 1.0);\n"
        "  texCoord = aTexCoord.xy;\n"
        "}",
        "#version 150 core\n"
        "out vec4 fragColor;\n"
        "in vec2 texCoord;\n"
        "uniform sampler2D texture0;\n"
        "void main()\n"
        "{\n"
        "  fragColor = texture(texture0, texCoord);\n"
        "}");
    shader_texture_with_color = compile_shader(
        "#version 150 core\n"
        "in vec2 aPos;\n"
        "in vec2 aTexCoord;\n"
        "in vec4 aColor;\n"
        "out vec2 texCoord;\n"
        "out vec4 outColor;\n"
        "void main()\n"
        "{\n"
        "  gl_Position = vec4(aPos, 0.0, 1.0);\n"
        "  texCoord = aTexCoord.xy;\n"
        "  outColor = aColor;\n"
        "}",
        "#version 150 core\n"
        "out vec4 fragColor;\n"
        "in vec2 texCoord;\n"
        "in vec4 outColor;\n"
        "uniform sampler2D texture0;\n"
        "void main()\n"
        "{\n"
        "  fragColor = texture(texture0, texCoord) * outColor;\n"
        "}");

    const float vertices[] = {
        // positions  // texture coords
        -1.f,  1.f,   0.0f, 1.0f, // top left
         1.f,  1.f,   1.0f, 1.0f, // top right
        -1.f, -1.f,   0.0f, 0.0f, // bottom left
         1.f, -1.f,   1.0f, 0.0f  // bottom right
    };
    const unsigned int indices[] = {
        0, 1, 2, // first triangle
        2, 1, 3  // second triangle
    };

    glGenVertexArrays(1, &vao_texture);
    glGenBuffers(1, &vbo_texture);
    glGenBuffers(1, &ebo_texture);

    glBindVertexArray(vao_texture);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_texture);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ebo_texture);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ebo_texture);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

}
