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
    Vec const& canvas_size();
}


// XXX

#include <vector>
inline std::vector<int> calculate_column_widths(std::vector<int> const& weights, int absolute = 0) {
    absolute = absolute ?: app::canvas_size().x;
    int relative = 0;
    for (int w : weights) {
        if (w > 0) {
            absolute -= w;
        }
        else {
            relative += -w;
        }
    }
    std::vector<int> widths;
    widths.reserve(weights.size());
    for (int w : weights) {
        if (w > 0) {
            widths.emplace_back(w);
        }
        else {
            int q = absolute * -w / relative;
            absolute -= q;
            relative += w;
            widths.emplace_back(q);
        }
    }
    return widths;
}

enum {
    BUTTON_BAR = 40,
    BUTTON_BIG = 30,
    BUTTON_SMALL = 20,
};
