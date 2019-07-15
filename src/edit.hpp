#pragma once
#include "gui.hpp"

enum EView {
    VIEW_PROJECT,
    VIEW_SONG,
    VIEW_TRACK,
    VIEW_INSTRUMENT,
    VIEW_EFFECT,
};

namespace edit {
    void set_view(EView v);
    void set_popup(void (*func)(void));

    void init();
    void draw();
    void free();
    void resize(int width, int height);
    Vec const& screen_size();
}


// XXX
#include <vector>
inline std::vector<int> calculate_column_widths(std::vector<int> const& weights, int absolute = 0) {
    absolute = absolute ? absolute : edit::screen_size().x;
    int relative = 0;
    for (int w : weights) {
        if (w > 0) absolute -= w;
        else       relative -= w;
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
    SCROLLBAR_WIDTH = 16,
};
