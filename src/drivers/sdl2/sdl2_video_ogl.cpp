#include "sdl2_video_ogl.h"

#include "sdl2_ttf_ogl.h"

#include "cfg.h"

#include "util.h"

#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <SDL.h>

namespace drivers {

using mat4f = float[16];

inline void gl_ortho_mat(mat4f &out, float left, float right, float bottom, float top, float znear, float zfar)
{
    out[0] = 2.0f / (right - left);
    out[1] = 0.0f;
    out[2] = 0.0f;
    out[3] = 0.0f;

    out[4] = 0.0f;
    out[5] = 2.0f / (top - bottom);
    out[6] = 0.0f;
    out[7] = 0.0f;

    out[8] = 0.0f;
    out[9] = 0.0f;
    out[10] = -2.0f / (zfar - znear);
    out[11] = 0.0f;

    out[12] = -(right + left) / (right - left);
    out[13] = -(top + bottom) / (top - bottom);
    out[14] = -(zfar + znear) / (zfar - znear);
    out[15] = 1.0f;
}

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
#ifdef USE_GLES
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    std::tie(curr_width, curr_height) = g_cfg.get_resolution();
    window = SDL_CreateWindow("SDLRetro", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              curr_width, curr_height, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);

    context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, context);

#ifdef USE_GLES
    gladLoadGLES2Loader(SDL_GL_GetProcAddress);
#else
    gladLoadGLLoader(SDL_GL_GetProcAddress);
#endif
    glViewport(0, 0, curr_width, curr_height);
    SDL_GL_SetSwapInterval(1);

    init_opengl();

    ttf[0] = std::make_shared<sdl2_ttf_ogl>(shader_font, vao_font, vbo_font, uniform_font_color);
    ttf[0]->init(16, 0);
    ttf[0]->add(g_cfg.get_data_dir() + PATH_SEPARATOR_CHAR + "fonts" + PATH_SEPARATOR_CHAR + "regular.ttf", 0);
    ttf[1] = std::make_shared<sdl2_ttf_ogl>(shader_font, vao_font, vbo_font, uniform_font_color);
    ttf[1]->init(16, 0);
    ttf[1]->add(g_cfg.get_data_dir() + PATH_SEPARATOR_CHAR + "fonts" + PATH_SEPARATOR_CHAR + "bold.ttf", 0);
}

sdl2_video_ogl::~sdl2_video_ogl() {
    uninit_opengl();
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
    if (width != game_width || height != game_height || pitch_in_pixel != game_pitch) {
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
        float sratio = (float)curr_width / curr_height;
        float wratio, hratio;
        if (g_cfg.get_integer_scaling()) {
            float int_ratio;
            if (sratio < ratio) {
                int_ratio = std::floor((float)curr_width / (float)width);
            } else {
                int_ratio = std::floor((float)curr_height / (float)height);
            }
            wratio = int_ratio * (float)width / curr_width;
            hratio = int_ratio * (float)height / curr_height;
        } else {
            if (sratio < ratio) {
                wratio = 1.f;
                hratio = (float)curr_width / ratio / curr_height;
            } else {
                wratio = ratio * (float)curr_height / curr_width;
                hratio = 1.f;
            }
        }
        float tratio = (float)width / game_pitch;
        glBindVertexArray(vao_texture);
        float vertices[] = {
            // positions        // texture coords
            -wratio,  hratio,   0.0f,   0.0f, // top left
             wratio,  hratio,   tratio, 0.0f, // top right
             wratio, -hratio,   tratio, 1.0f, // bottom right
            -wratio, -hratio,   0.0f,   1.0f  // bottom left
        };
        glBindBuffer(GL_ARRAY_BUFFER, vbo_texture);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, texture_game);
        switch (game_pixel_format) {
        case 0:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, game_pitch, height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_5_5_1, nullptr);
            break;
        case 1:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, game_pitch, height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, nullptr);
            break;
        default:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, game_pitch, height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, nullptr);
            break;
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_game);
    switch (game_pixel_format) {
    case 0:
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, game_pitch, height, GL_RGB, GL_UNSIGNED_SHORT_5_5_5_1, data);
        break;
    case 1:
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, game_pitch, height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
        break;
    default:
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, game_pitch, height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, data);
        break;
    }
    do_render();
    SDL_GL_SwapWindow(window);
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
    draw_color[0] = (float)r / 255.f;
    draw_color[1] = (float)g / 255.f;
    draw_color[2] = (float)b / 255.f;
    draw_color[3] = (float)a / 255.f;
}

void sdl2_video_ogl::draw_rectangle(int x, int y, int w, int h) {
    auto x1 = (float)x - 0.5f, y1 = (float)y - 0.5f, x2 = (float)(x + w) + 0.5f, y2 = (float)(y + h) + 0.5f;
    float vertices[] = {
        x1, y1, draw_color[0], draw_color[1], draw_color[2], draw_color[3],
        x2, y1, draw_color[0], draw_color[1], draw_color[2], draw_color[3],
        x2, y2, draw_color[0], draw_color[1], draw_color[2], draw_color[3],
        x1, y2, draw_color[0], draw_color[1], draw_color[2], draw_color[3]
    };
    glUseProgram(shader_direct_draw);
    glBindVertexArray(vao_draw);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_draw);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glBindVertexArray(0);
    glUseProgram(0);
}

void sdl2_video_ogl::fill_rectangle(int x, int y, int w, int h) {
    auto x1 = (float)x, y1 = (float)y, x2 = (float)(x + w), y2 = (float)(y + h);
    float vertices[] = {
        x1, y1, draw_color[0], draw_color[1], draw_color[2], draw_color[3],
        x2, y1, draw_color[0], draw_color[1], draw_color[2], draw_color[3],
        x1, y2, draw_color[0], draw_color[1], draw_color[2], draw_color[3],
        x2, y2, draw_color[0], draw_color[1], draw_color[2], draw_color[3]
    };
    glUseProgram(shader_direct_draw);
    glBindVertexArray(vao_draw);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_draw);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glUseProgram(0);
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

void sdl2_video_ogl::predraw_menu() {
    glUseProgram(shader_texture);
    glBindVertexArray(vao_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_game);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    set_draw_color(0, 0, 0, 0xA0);
    fill_rectangle(0, 0, curr_width, curr_height);
}

void sdl2_video_ogl::config_changed() {
    glBindTexture(GL_TEXTURE_2D, texture_game);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, g_cfg.get_linear() ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, g_cfg.get_linear() ? GL_LINEAR : GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    game_width = game_height = game_pitch = 0;
}

void sdl2_video_ogl::do_render() {
    clear();

    glUseProgram(shader_texture);
    glBindVertexArray(vao_texture);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    if (messages.empty()) return;
    uint32_t lh = ttf[0]->get_font_size() + 2;
    uint32_t y = curr_height - 5 - (messages.size() - 1) * lh;
    for (auto &m: messages) {
        draw_text(5, y, m.first.c_str(), 0, true);
        y += lh;
    }
}

void sdl2_video_ogl::init_opengl() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifdef USE_GLES
#define GLSL_VERSION_STR "300 es"
#else
#define GLSL_VERSION_STR "150 core"
#endif
    shader_direct_draw = compile_shader(
        "#version " GLSL_VERSION_STR "\n"
        "in vec2 aPos;\n"
        "in vec4 aColor;\n"
        "out vec4 outColor;\n"
        "uniform mat4 projMat;\n"
        "void main()\n"
        "{\n"
        "  gl_Position = projMat * vec4(aPos, 0.0, 1.0);\n"
        "  outColor = aColor;\n"
        "}",
        "#version " GLSL_VERSION_STR "\n"
        "#ifdef GL_ES\n"
        "precision mediump float;\n"
        "#endif\n"
        "out vec4 fragColor;\n"
        "in vec4 outColor;\n"
        "void main()\n"
        "{\n"
        "  fragColor = outColor;\n"
        "}");
    shader_texture = compile_shader(
        "#version " GLSL_VERSION_STR "\n"
        "in vec2 aPos;\n"
        "in vec2 aTexCoord;\n"
        "out vec2 texCoord;\n"
        "void main()\n"
        "{\n"
        "  gl_Position = vec4(aPos, 0.0, 1.0);\n"
        "  texCoord = aTexCoord.xy;\n"
        "}",
        "#version " GLSL_VERSION_STR "\n"
        "#ifdef GL_ES\n"
        "precision mediump float;\n"
        "#endif\n"
        "out vec4 fragColor;\n"
        "in vec2 texCoord;\n"
        "uniform sampler2D texture0;\n"
        "void main()\n"
        "{\n"
        "  fragColor = texture(texture0, texCoord);\n"
        "}");
    shader_font = compile_shader(
        "#version " GLSL_VERSION_STR "\n"
        "in vec2 aPos;\n"
        "in vec2 aTexCoord;\n"
        "out vec2 texCoord;\n"
        "uniform mat4 projMat;\n"
        "void main()\n"
        "{\n"
        "  gl_Position = projMat * vec4(aPos, 0.0, 1.0);\n"
        "  texCoord = aTexCoord.xy;\n"
        "}",
        "#version " GLSL_VERSION_STR "\n"
        "#ifdef GL_ES\n"
        "precision mediump float;\n"
        "#endif\n"
        "out vec4 fragColor;\n"
        "in vec2 texCoord;\n"
        "uniform sampler2D texture0;\n"
        "uniform vec3 outColor;\n"
        "void main()\n"
        "{\n"
        "  fragColor = vec4(outColor, texture(texture0, texCoord).r);\n"
        "}");

    glGenVertexArrays(1, &vao_draw);
    glGenBuffers(1, &vbo_draw);

    const unsigned int indices[] = {
        0, 1, 2, // first triangle
        0, 2, 3  // second triangle
    };
    glGenVertexArrays(1, &vao_texture);
    glGenBuffers(1, &vbo_texture);
    glGenBuffers(1, &ebo_texture);
    glBindVertexArray(vao_texture);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_texture);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindVertexArray(0);

    glGenVertexArrays(1, &vao_font);
    glGenBuffers(1, &vbo_font);
    glGenBuffers(1, &ebo_font);
    glBindVertexArray(vao_font);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_font);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindVertexArray(0);

    glGenTextures(1, &texture_game);
    glBindTexture(GL_TEXTURE_2D, texture_game);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, g_cfg.get_linear() ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, g_cfg.get_linear() ? GL_LINEAR : GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(shader_texture);
    glUniform1i(glGetUniformLocation(shader_texture, "texture0"), 0);
    glUseProgram(0);

    glUseProgram(shader_font);
    glUniform1i(glGetUniformLocation(shader_font, "texture0"), 0);
    glUseProgram(0);

    mat4f proj_mat;
    gl_ortho_mat(proj_mat, 0.0f, (float)curr_width, (float)curr_height, 0.0f, 0.0f, 1.0f);
    glUseProgram(shader_direct_draw);
    glUniformMatrix4fv(glGetUniformLocation(shader_direct_draw, "projMat"), 1, GL_FALSE, proj_mat);
    glUseProgram(0);
    glUseProgram(shader_font);
    glUniformMatrix4fv(glGetUniformLocation(shader_font, "projMat"), 1, GL_FALSE, proj_mat);
    uniform_font_color = glGetUniformLocation(shader_font, "outColor");
    glUseProgram(0);
}

void sdl2_video_ogl::uninit_opengl() {
    glDeleteTextures(1, &texture_game);
    texture_game = 0;

    glDeleteBuffers(1, &ebo_font);
    ebo_font = 0;
    glDeleteBuffers(1, &vbo_font);
    vbo_font = 0;
    glDeleteVertexArrays(1, &vao_font);
    vao_font = 0;

    glDeleteBuffers(1, &ebo_texture);
    ebo_texture = 0;
    glDeleteBuffers(1, &vbo_texture);
    vbo_texture = 0;
    glDeleteVertexArrays(1, &vao_texture);
    vao_texture = 0;

    glDeleteBuffers(1, &vbo_draw);
    vbo_draw = 0;
    glDeleteVertexArrays(1, &vao_draw);
    vao_draw = 0;

    glDeleteProgram(shader_font);
    shader_font = 0;
    glDeleteProgram(shader_texture);
    shader_texture = 0;
    glDeleteProgram(shader_direct_draw);
    shader_direct_draw = 0;
}

}
