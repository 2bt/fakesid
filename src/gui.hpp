#pragma once
#include "render.hpp"


enum {
    KEYCODE_ENTER = 66,
    KEYCODE_DEL   = 67,
};


namespace gui {
    enum {
        FONT_WIDTH  = 8,
        FONT_HEIGHT = 8,
        SEPARATOR_WIDTH = 2,
        CLAVIER_WIDTH   = 24,
    };

    struct Box {
        bool contains(Vec const& p) const {
            return p.x >= pos.x && p.y >= pos.y &&
                   p.x < pos.x + size.x && p.y < pos.y + size.y;
        }
        Vec pos;
        Vec size;
    };


    enum Align { A_CENTER, A_LEFT, A_RIGHT };

    enum Icon {
        I_LOOP,
        I_STOP,
        I_PLAY,
        I_COPY,
        I_PASTE,
        I_LOWPASS,
        I_BANDPASS,
        I_HIGHPASS,
        I_NOISE,
        I_PULSE,
        I_SAW,
        I_TRI,
        I_RING,
        I_SYNC,
        I_GATE,
        I_EMPTY,
        I_ADD_ROW_ABOVE,
        I_ADD_ROW_BELOW,
        I_DELETE_ROW,
    };

    enum EDragTheme {
        DT_NORMAL,
        DT_SCROLLBAR,
    };
    enum EButtonTheme {
        BT_NORMAL,
        BT_HIGHLIGHT,
        BT_CURSOR,
        BT_TAB,
        BT_TAB_HIGHLIGHT,
        BT_JAM,
    };


    Vec  cursor();
    void cursor(Vec const& c);
    void id(void const* addr);
    void same_line();
    void next_line();
    void align(Align a);
    void min_item_size(Vec const& s = {});
    void begin_frame();
    Box  padding(Vec const& size);
    void separator();
    void text(char const* fmt, ...);
    bool button(char const* label, bool active = false);
    bool button(Icon icon, bool active = false);
    bool hold();
    void input_text(char* str, int len);
    bool drag_int(char const* label, char const* fmt, int& value, int min, int max, int page = 0);
    bool vertical_drag_int(int& value, int min, int max, int page = 1);
    bool clavier(uint8_t& n, int offset);
    void dumb_button(int state);

    template<class T>
    bool drag_int(char const* label, char const* fmt, T& value, int min, int max, int page = 0) {
        int v = value;
        id(&value);
        bool b = drag_int(label, fmt, v, min, max, page);
        if (b) value = v;
        return b;
    }

    struct Touch {
        bool just_pressed()  const { return pressed && !prev_pressed; }
        bool just_released() const { return !pressed && prev_pressed; }
        bool box_touched(Box const& box) const { return (pressed | prev_pressed) && box.contains(pos); }
        bool pressed;
        bool prev_pressed;
        Vec  pos;
        Vec  prev_pos;
    };

    Touch const& touch();

    void touch(int x, int y, bool pressed);
    void key(int key, int unicode);
    void init();
    void free();
    void render(gfx::RenderTarget* rt);

    void button_theme(EButtonTheme bt);
    void drag_theme(EDragTheme bt);
}

