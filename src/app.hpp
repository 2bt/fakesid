#pragma once
#include "gui.hpp"

namespace app {
    void init();
    void free();
    void resize(int width, int height);
    void draw();
    void exit();
    void touch(int x, int y, bool pressed);
    void key(int key, int unicode);
}
