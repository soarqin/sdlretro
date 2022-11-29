#include "sdl2_video.h"

#include "sdl2_ttf.h"

#include "driver_base.h"

#include "logger.h"
#include "cfg.h"

#include "helper.h"

#include "core.h"

#include <glad/glad.h>
#include <SDL.h>

namespace drivers {

inline uint32_t compile_shader(const std::string &vertex_shader_source,
                               const std::string &fragment_shader_source) {
    uint32_t vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *src = vertex_shader_source.c_str();
    glShaderSource(vertex_shader, 1, &src, nullptr);
    glCompileShader(vertex_shader);
    // check for shader compile errors
    int success;
    char info_log[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
        LOG(ERROR, "vertex shader compilation failed: {}", info_log);
        glDeleteShader(vertex_shader);
        return 0;
    }
    // fragment shader
    uint32_t fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    src = fragment_shader_source.c_str();
    glShaderSource(fragment_shader, 1, &src, nullptr);
    glCompileShader(fragment_shader);
    // check for shader compile errors
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log);
        LOG(ERROR, "fragment shader compilation failed: {}", info_log);
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
        LOG(ERROR, "shader program linking failed: {}", info_log);
        glDeleteProgram(shader_program);
        shader_program = 0;
    }
    glDeleteShader(fragment_shader);
    glDeleteShader(vertex_shader);
    return shader_program;
}

sdl2_video::sdl2_video(): saved_x(SDL_WINDOWPOS_CENTERED), saved_y(SDL_WINDOWPOS_CENTERED) {
    if (
#if defined(_WIN32) || (defined(__APPLE__) && !defined(IPHONE) && defined(__MACH__))
        /* Prefer OpenGL to GLES on Windows and macOS */
        !init_video(false) && !init_video(true)
#else
        /* Prefer GLES to OpenGL on other platforms */
        !init_video(true) && !init_video(false)
#endif
        ) {
        throw std::bad_exception();
    }
}

sdl2_video::~sdl2_video() {
    deinit_video();
}

int sdl2_video::get_renderer_type() {
    return gl_renderer.use_gles ? 2 : 1;
}

inline unsigned pullup(unsigned n) {
    n |= n >> 1U;
    n |= n >> 2U;
    n |= n >> 4U;
    n |= n >> 8U;
    return (n | (n >> 16U)) + 1U;
}

extern driver_base *current_driver;

extern "C" {

uintptr_t RETRO_CALLCONV hw_get_current_framebuffer() {
    return static_cast<sdl2_video*>(current_driver->get_video())->get_hw_fbo();
}

retro_proc_address_t RETRO_CALLCONV hw_get_proc_address(const char *sym) {
    return (retro_proc_address_t)SDL_GL_GetProcAddress(sym);
}

}

bool sdl2_video::init_hw_renderer(retro_hw_render_callback *hwr) {
    deinit_hw_renderer();

    if (hwr->context_type == RETRO_HW_CONTEXT_OPENGLES3 || hwr->context_type == RETRO_HW_CONTEXT_OPENGLES_VERSION) {
        if (!init_video(true)) {
            init_video(false);
            return false;
        }
    } else if (hwr->context_type == RETRO_HW_CONTEXT_OPENGL || hwr->context_type == RETRO_HW_CONTEXT_OPENGL_CORE) {
        if (!init_video(false)) {
            init_video(true);
            return false;
        }
    } else {
        return false;
    }

    gl_renderer_update_texture_rect(true);

    glGenFramebuffers(1, &hw_renderer.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, hw_renderer.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gl_renderer.texture_game, 0);
    hw_renderer.rb_ds = 0;
    gl_renderer.bottom_left = hwr->bottom_left_origin;
    if (hwr->depth) {
        glGenRenderbuffers(1, &hw_renderer.rb_ds);
        glBindRenderbuffer(GL_RENDERBUFFER, hw_renderer.rb_ds);
        glRenderbufferStorage(GL_RENDERBUFFER, hwr->stencil ? GL_DEPTH24_STENCIL8 : GL_DEPTH_COMPONENT16,
                              gl_renderer.texture_w, gl_renderer.texture_h);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        if (hwr->stencil)
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, hw_renderer.rb_ds);
        else
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hw_renderer.rb_ds);
    }
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        LOG(ERROR, "Framebuffer is not complete.");
        return false;
    }

    if (hwr->depth && hwr->stencil)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    else if (hwr->depth)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    else
        glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    hwr->get_current_framebuffer = hw_get_current_framebuffer;
    hwr->get_proc_address = hw_get_proc_address;
    hwr_cb = hwr;
    return true;
}

void sdl2_video::inited_hw_renderer() {
    if (hwr_cb) {
        hwr_cb->context_reset();
    }
}

void sdl2_video::deinit_hw_renderer() {
    if (hw_renderer.rb_ds) {
        glDeleteRenderbuffers(1, &hw_renderer.rb_ds);
        hw_renderer.rb_ds = 0;
    }
    if (hw_renderer.fbo) {
        glDeleteFramebuffers(1, &hw_renderer.fbo);
        hw_renderer.fbo = 0;
    }
    if (hwr_cb) {
        hwr_cb->context_destroy();
        hwr_cb = nullptr;
    }
}

void sdl2_video::window_resized(int width, int height, bool fullscreen) {
    if (fullscreen) {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        SDL_DisplayMode mode = {};
        SDL_GetWindowDisplayMode(window, &mode);
        curr_width = mode.w;
        curr_height = mode.h;
    } else {
        SDL_SetWindowFullscreen(window, 0);
        SDL_SetWindowSize(window, width, height);
        curr_width = width;
        curr_height = height;
    }
    gl_set_ortho();
    init_fonts();
    recalc_draw_rect();
}

bool sdl2_video::game_resolution_changed(int width, int height, int max_width, int max_height, uint32_t pixel_format) {
    game_width = width;
    game_height = height;
    game_max_width = max_width;
    game_max_height = max_height;
    bool pixel_format_changed = pixel_format != game_pixel_format;
    if (pixel_format_changed) {
        game_pixel_format = pixel_format;
        bpp = pixel_format == 1 ? 4 : 2;
    }
    recalc_draw_rect(pixel_format_changed);
    return true;
}

void sdl2_video::render(const void *data, int width, int height, size_t pitch) {
    if (skip_frame) {
        drawn = false;
        skip_frame = false;
        return;
    }
    if (width != game_width || height != game_height) {
        game_width = width;
        game_height = height;
        if (!recalc_draw_rect()) {
            drawn = false;
            return;
        }
    }
    if (data != nullptr && data != RETRO_HW_FRAME_BUFFER_VALID) {
        if (!gl_renderer_gen_texture(data, pitch / bpp)) {
            drawn = false;
            return;
        }
    }
    drawn = true;
}

void sdl2_video::frame_render() {
    clear();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, curr_width, curr_height);
    glUseProgram(gl_renderer.program_texture);
    glBindVertexArray(gl_renderer.vao_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gl_renderer.texture_game);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if (!messages.empty()) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        uint32_t lh = ttf[0]->get_font_size() + 2;
        uint32_t y = curr_height - 5 - (messages.size() - 1) * lh;
        for (auto &m: messages) {
            draw_text(5, y, m.first.c_str(), 0, true);
            y += lh;
        }
    }
    flip();
}

void *sdl2_video::get_framebuffer(uint32_t *width, uint32_t *height, size_t *pitch, int *format) {
    return video_base::get_framebuffer(width, height, pitch, format);
}

void sdl2_video::clear() {
    glClearColor(0.f, 0.f, 0.f, 1.f);
    if (hwr_cb) {
        if (hwr_cb->depth && hwr_cb->stencil)
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        else if (hwr_cb->depth)
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        else
            glClear(GL_COLOR_BUFFER_BIT);
    } else
        glClear(GL_COLOR_BUFFER_BIT);
}

void sdl2_video::flip() {
    SDL_GL_SwapWindow(window);
}

int sdl2_video::get_font_size() const {
    return ttf[0]->get_font_size();
}

void sdl2_video::set_draw_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    gl_renderer.draw_color[0] = (float)r / 255.f;
    gl_renderer.draw_color[1] = (float)g / 255.f;
    gl_renderer.draw_color[2] = (float)b / 255.f;
    gl_renderer.draw_color[3] = (float)a / 255.f;
}

void sdl2_video::draw_rectangle(int x, int y, int w, int h) {
    auto x1 = (float)x - 0.5f, y1 = (float)y - 0.5f, x2 = (float)(x + w) + 0.5f, y2 = (float)(y + h) + 0.5f;
    float vertices[] = {
        x1, y1, gl_renderer.draw_color[0], gl_renderer.draw_color[1], gl_renderer.draw_color[2], gl_renderer.draw_color[3],
        x2, y1, gl_renderer.draw_color[0], gl_renderer.draw_color[1], gl_renderer.draw_color[2], gl_renderer.draw_color[3],
        x2, y2, gl_renderer.draw_color[0], gl_renderer.draw_color[1], gl_renderer.draw_color[2], gl_renderer.draw_color[3],
        x1, y2, gl_renderer.draw_color[0], gl_renderer.draw_color[1], gl_renderer.draw_color[2], gl_renderer.draw_color[3]
    };
    glUseProgram(gl_renderer.program_direct_draw);
    glBindVertexArray(gl_renderer.vao_draw);
    glBindBuffer(GL_ARRAY_BUFFER, gl_renderer.vbo_draw);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glBindVertexArray(0);
    glUseProgram(0);
}

void sdl2_video::fill_rectangle(int x, int y, int w, int h) {
    auto x1 = (float)x, y1 = (float)y, x2 = (float)(x + w), y2 = (float)(y + h);
    float vertices[] = {
        x1, y1, gl_renderer.draw_color[0], gl_renderer.draw_color[1], gl_renderer.draw_color[2], gl_renderer.draw_color[3],
        x2, y1, gl_renderer.draw_color[0], gl_renderer.draw_color[1], gl_renderer.draw_color[2], gl_renderer.draw_color[3],
        x1, y2, gl_renderer.draw_color[0], gl_renderer.draw_color[1], gl_renderer.draw_color[2], gl_renderer.draw_color[3],
        x2, y2, gl_renderer.draw_color[0], gl_renderer.draw_color[1], gl_renderer.draw_color[2], gl_renderer.draw_color[3]
    };
    glUseProgram(gl_renderer.program_direct_draw);
    glBindVertexArray(gl_renderer.vao_draw);
    glBindBuffer(GL_ARRAY_BUFFER, gl_renderer.vbo_draw);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glUseProgram(0);
}

void sdl2_video::draw_text(int x, int y, const char *text, int width, bool shadow) {
    if (width == 0) width = curr_width - x;
    else if (width < 0) width = x - curr_width;
    ttf[0]->render(x, y, text, width, curr_height + ttf[0]->get_font_size() - y, shadow);
}

void sdl2_video::get_text_width_and_height(const char *text, int &w, int &t, int &b) const {
    w = 0;
    t = 255;
    b = -255;
    while (*text != 0) {
        uint32_t ch = helper::utf8_to_ucs4(text);
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

void sdl2_video::gui_predraw() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(gl_renderer.program_texture);
    glBindVertexArray(gl_renderer.vao_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gl_renderer.texture_game);
    glViewport(0, 0, curr_width, curr_height);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    set_draw_color(0, 0, 0, 0xA0);
    fill_rectangle(0, 0, curr_width, curr_height);
}

void sdl2_video::config_changed() {
    glBindTexture(GL_TEXTURE_2D, gl_renderer.texture_game);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, g_cfg.get_linear() ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, g_cfg.get_linear() ? GL_LINEAR : GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    game_width = game_height = 0;
}

bool sdl2_video::init_video(bool use_gles) {
    deinit_video();

    if (use_gles) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    } else {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    }
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    if (g_cfg.get_fullscreen()) {
        window = SDL_CreateWindow("SDLRetro", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0,
                                  SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
        if (window == nullptr) return false;
        SDL_DisplayMode mode = {};
        SDL_GetWindowDisplayMode(window, &mode);
        curr_width = mode.w;
        curr_height = mode.h;
    } else {
        g_cfg.get_resolution(curr_width, curr_height);
        window = SDL_CreateWindow("SDLRetro", saved_x, saved_y, curr_width, curr_height,
                                  SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
        if (window == nullptr) return false;
    }

    gl_renderer.use_gles = use_gles;
    context = SDL_GL_CreateContext(window);
    if (context == nullptr) {
        SDL_DestroyWindow(window);
        window = nullptr;
        return false;
    }
    SDL_GL_MakeCurrent(window, context);

    if (gl_renderer.use_gles) {
        gladLoadGLES2Loader(SDL_GL_GetProcAddress);
    } else {
        gladLoadGLLoader(SDL_GL_GetProcAddress);
    }

    LOG(TRACE, "Created window with OpenGL context");
    LOG(TRACE, "  GL version: {}", (const char*)glGetString(GL_VERSION));
    LOG(TRACE, "  Renderer: {}", (const char*)glGetString(GL_RENDERER));
    LOG(TRACE, "  Shading Language version: {}", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    glViewport(0, 0, curr_width, curr_height);
    /* Try adaptive vsync first, and fallthrough to normal vsync */
    if (SDL_GL_SetSwapInterval(-1) < 0) {
        SDL_GL_SetSwapInterval(1);
    }

    init_opengl();
    init_fonts();

    return true;
}

void sdl2_video::deinit_video() {
    if (ttf[0]) {
        ttf[0].reset();
    }
    if (ttf[1]) {
        ttf[1].reset();
    }
    deinit_opengl();
    if (context) {
        SDL_GL_DeleteContext(context);
        context = nullptr;
    }
    if (window) {
        if (!g_cfg.get_fullscreen()) {
            SDL_GetWindowPosition(window, &saved_x, &saved_y);
        }
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}

void sdl2_video::init_fonts() {
    if (ttf[0]) {
        ttf[0]->deinit();
    } else {
        ttf[0] = std::make_shared<sdl2_ttf>(gl_renderer.program_font, gl_renderer.vao_font, gl_renderer.vbo_font, gl_renderer.uniform_font_color);
    }
    if (ttf[1]) {
        ttf[1]->deinit();
    } else {
        ttf[1] = std::make_shared<sdl2_ttf>(gl_renderer.program_font, gl_renderer.vao_font, gl_renderer.vbo_font, gl_renderer.uniform_font_color);
    }
    auto size = std::min(16 * curr_width / 640, 16 * curr_height / 480) & ~1u;
    ttf[0]->init(size, 0);
    ttf[0]->add(g_cfg.get_data_dir() + PATH_SEPARATOR_CHAR + "fonts" + PATH_SEPARATOR_CHAR + "regular.ttf", 0);
    ttf[1]->init(size, 0);
    ttf[1]->add(g_cfg.get_data_dir() + PATH_SEPARATOR_CHAR + "fonts" + PATH_SEPARATOR_CHAR + "bold.ttf", 0);
}

void sdl2_video::init_opengl() {
    std::string glsl_version_str = gl_renderer.use_gles ? "300 es" : "330 core";
    gl_renderer.program_direct_draw = compile_shader(
        "#version " + glsl_version_str + "\n"
        "in vec2 aPos;\n"
        "in vec4 aColor;\n"
        "out vec4 outColor;\n"
        "uniform mat4 projMat;\n"
        "void main()\n"
        "{\n"
        "  gl_Position = projMat * vec4(aPos, 0.0, 1.0);\n"
        "  outColor = aColor;\n"
        "}",
        "#version " + glsl_version_str + "\n"
        "#ifdef GL_ES\n"
        "precision mediump float;\n"
        "#endif\n"
        "out vec4 fragColor;\n"
        "in vec4 outColor;\n"
        "void main()\n"
        "{\n"
        "  fragColor = outColor;\n"
        "}");
    gl_renderer.program_texture = compile_shader(
        "#version " + glsl_version_str + "\n"
        "in vec2 aPos;\n"
        "in vec2 aTexCoord;\n"
        "out vec2 texCoord;\n"
        "void main()\n"
        "{\n"
        "  gl_Position = vec4(aPos, 0.0, 1.0);\n"
        "  texCoord = aTexCoord.xy;\n"
        "}",
        "#version " + glsl_version_str + "\n"
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
    gl_renderer.program_font = compile_shader(
        "#version " + glsl_version_str + "\n"
        "in vec2 aPos;\n"
        "in vec2 aTexCoord;\n"
        "out vec2 texCoord;\n"
        "uniform mat4 projMat;\n"
        "void main()\n"
        "{\n"
        "  gl_Position = projMat * vec4(aPos, 0.0, 1.0);\n"
        "  texCoord = aTexCoord.xy;\n"
        "}",
        "#version " + glsl_version_str + "\n"
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

    gl_renderer.bottom_left = false;
    glGenVertexArrays(1, &gl_renderer.vao_draw);
    glGenBuffers(1, &gl_renderer.vbo_draw);

    glGenVertexArrays(1, &gl_renderer.vao_texture);
    glGenBuffers(1, &gl_renderer.vbo_texture);

    glGenVertexArrays(1, &gl_renderer.vao_font);
    glGenBuffers(1, &gl_renderer.vbo_font);

    glGenTextures(1, &gl_renderer.texture_game);
    glBindTexture(GL_TEXTURE_2D, gl_renderer.texture_game);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, g_cfg.get_linear() ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, g_cfg.get_linear() ? GL_LINEAR : GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(gl_renderer.program_texture);
    glUniform1i(glGetUniformLocation(gl_renderer.program_texture, "texture0"), 0);
    glUseProgram(0);

    glUseProgram(gl_renderer.program_font);
    glUniform1i(glGetUniformLocation(gl_renderer.program_font, "texture0"), 0);
    glUseProgram(0);

    gl_set_ortho();
}

void sdl2_video::deinit_opengl() {
    if (gl_renderer.texture_game) {
        glDeleteTextures(1, &gl_renderer.texture_game);
        gl_renderer.texture_game = 0;
    }

    if (gl_renderer.vbo_font) {
        glDeleteBuffers(1, &gl_renderer.vbo_font);
        gl_renderer.vbo_font = 0;
    }
    if (gl_renderer.vao_font) {
        glDeleteVertexArrays(1, &gl_renderer.vao_font);
        gl_renderer.vao_font = 0;
    }

    if (gl_renderer.vbo_texture) {
        glDeleteBuffers(1, &gl_renderer.vbo_texture);
        gl_renderer.vbo_texture = 0;
    }
    if (gl_renderer.vao_texture) {
        glDeleteVertexArrays(1, &gl_renderer.vao_texture);
        gl_renderer.vao_texture = 0;
    }

    if (gl_renderer.vbo_draw) {
        glDeleteBuffers(1, &gl_renderer.vbo_draw);
        gl_renderer.vbo_draw = 0;
    }
    if (gl_renderer.vao_draw) {
        glDeleteVertexArrays(1, &gl_renderer.vao_draw);
        gl_renderer.vao_draw = 0;
    }

    if (gl_renderer.program_font) {
        glDeleteProgram(gl_renderer.program_font);
        gl_renderer.program_font = 0;
    }
    if (gl_renderer.program_texture) {
        glDeleteProgram(gl_renderer.program_texture);
        gl_renderer.program_texture = 0;
    }
    if (gl_renderer.program_direct_draw) {
        glDeleteProgram(gl_renderer.program_direct_draw);
        gl_renderer.program_direct_draw = 0;
    }
    gl_renderer.bottom_left = false;
}

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

void sdl2_video::gl_set_ortho() {
    mat4f proj_mat;
    gl_ortho_mat(proj_mat, 0.0f, (float)curr_width, (float)curr_height, 0.0f, 0.0f, 1.0f);
    glUseProgram(gl_renderer.program_direct_draw);
    glUniformMatrix4fv(glGetUniformLocation(gl_renderer.program_direct_draw, "projMat"), 1, GL_FALSE, proj_mat);
    glUseProgram(0);
    glUseProgram(gl_renderer.program_font);
    glUniformMatrix4fv(glGetUniformLocation(gl_renderer.program_font, "projMat"), 1, GL_FALSE, proj_mat);
    gl_renderer.uniform_font_color = glGetUniformLocation(gl_renderer.program_font, "outColor");
    glUseProgram(0);
}

bool sdl2_video::recalc_draw_rect(bool force_create_empty_texture) {
    gl_renderer_update_texture_rect(force_create_empty_texture);

    float ratio = (float)game_width / (float)game_height;
    /* TODO: aspect_ratio from geometry is bad sometimes,
     *       so we just use w/h here */
    float sratio = (float)curr_width / (float)curr_height;
    float wratio, hratio;
    if (g_cfg.get_integer_scaling()) {
        float int_ratio;
        if (sratio < ratio) {
            int_ratio = std::floor((float)curr_width / (float)game_width);
        } else {
            int_ratio = std::floor((float)curr_height / (float)game_height);
        }
        wratio = int_ratio * (float)game_width / (float)curr_width;
        hratio = int_ratio * (float)game_height / (float)curr_height;
    } else {
        if (sratio < ratio) {
            wratio = 1.f;
            hratio = (float)curr_width / ratio / (float)curr_height;
        } else {
            wratio = ratio * (float)curr_height / (float)curr_width;
            hratio = 1.f;
        }
    }
    return gl_renderer_resized(wratio, hratio);
}

void sdl2_video::gl_renderer_update_texture_rect(bool force_create_empty_texture) {
    auto new_w = pullup(game_max_width);
    auto new_h = pullup(game_max_height);
    if (force_create_empty_texture || new_w != gl_renderer.texture_w || new_h != gl_renderer.texture_h) {
        gl_renderer.texture_w = new_w;
        gl_renderer.texture_h = new_h;
        gl_renderer_create_empty_texture();
    }
}

void sdl2_video::gl_renderer_create_empty_texture() const {
    glBindTexture(GL_TEXTURE_2D, gl_renderer.texture_game);
    switch (game_pixel_format) {
    case 0:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gl_renderer.texture_w, gl_renderer.texture_h, 0, GL_BGRA, GL_UNSIGNED_SHORT_5_5_5_1, nullptr);
        break;
    case 1:
        if (gl_renderer.use_gles) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gl_renderer.texture_w, gl_renderer.texture_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_BLUE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ONE);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gl_renderer.texture_w, gl_renderer.texture_h, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, nullptr);
        }
        break;
    default:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gl_renderer.texture_w, gl_renderer.texture_h, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, nullptr);
        break;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool sdl2_video::gl_renderer_resized(float wratio, float hratio) const {
    float o_r, o_b;
    o_r = (float)game_width / gl_renderer.texture_w;
    o_b = (float)game_height / gl_renderer.texture_h;
    glBindVertexArray(gl_renderer.vao_texture);
    if (gl_renderer.bottom_left) {
        float vertices[] = {
            // positions      // texture coords
            -wratio, hratio,  0.f, o_b, // top left
             wratio, hratio,  o_r, o_b, // top right
            -wratio, -hratio, 0.f, 0.f, // bottom left
             wratio, -hratio, o_r, 0.f  // bottom right
        };
        glBindBuffer(GL_ARRAY_BUFFER, gl_renderer.vbo_texture);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    } else {
        float vertices[] = {
            // positions      // texture coords
            -wratio, hratio,  0.f, 0.f, // top left
             wratio, hratio,  o_r, 0.f, // top right
            -wratio, -hratio, 0.f, o_b, // bottom left
             wratio, -hratio, o_r, o_b // bottom right
        };
        glBindBuffer(GL_ARRAY_BUFFER, gl_renderer.vbo_texture);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    return true;
}

bool sdl2_video::gl_renderer_gen_texture(const void *data, size_t pitch) const {
    if (!data) {
        return false;
    }
    glBindTexture(GL_TEXTURE_2D, gl_renderer.texture_game);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, pitch);
    glBlendFunc(GL_ONE, GL_ZERO);
    switch (game_pixel_format) {
    case 0:
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, game_width, game_height, GL_BGRA, GL_UNSIGNED_SHORT_5_5_5_1, data);
        break;
    case 1:
        if (gl_renderer.use_gles) {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, game_width, game_height, GL_RGBA, GL_UNSIGNED_BYTE, data);
        } else {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, game_width, game_height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
        }
        break;
    default:
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, game_width, game_height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, data);
        break;
    }
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

}
