#pragma once
#include <glm/glm.hpp>

union SDL_Event;

namespace gui {

    using Vec = glm::i16vec2;

    void init();
    void resize(int width, int height);
    void free();

    bool process_event(const SDL_Event& e);
    void new_frame();
    void render();

    void set_next_window_pos(const Vec& pos);
    void begin_window(const char* name);
    void end_window();

    void same_line(short offset = 0);
    void separator();

    void text(const char* fmt, ...);
    bool button(const char* label);
    bool checkbox(const char* label, bool& v);
    bool radio_button(const char* label, int& v, int value);
    bool drag_float(const char* label, float& v, float speed = 1, float min = 0, float max = 0, const char* fmt = "%.3f");
}
