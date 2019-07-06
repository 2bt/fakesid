#pragma once

namespace app {
    bool init();
    void free();
    void resize(int width, int height);
    void draw();
    void exit();
    void touch(int x, int y);
}
