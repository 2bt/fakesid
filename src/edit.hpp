#pragma once

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
}
