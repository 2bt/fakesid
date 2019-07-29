#pragma once
#include "gui.hpp"

namespace app {
    enum {
        WIDTH      = 360,
        MIN_HEIGHT = 590,
    };

    void init();
    void free();
    void resize(int width, int height);
    void draw();
    void touch(int x, int y, bool pressed);
    void key(int key, int unicode);
}
