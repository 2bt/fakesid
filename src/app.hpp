#pragma once
#include "gfx.hpp"

class App {
public:
    bool init();
    void free();
    void resize(int width, int height);
    void draw();
    void exit();
    void touch(int x, int y);

private:
    bool m_initialized = false;

    int m_width;
    int m_height;

    int m_touch_x;
    int m_touch_y;

    gfx::Texture2D*   m_canvas;
    gfx::Framebuffer* m_framebuffer;

};
