#pragma once

#define SETTINGS(X) \
    X(fullscreen_enabled,   int,  0) \
    X(play_in_background,   int,  0) \
    X(track_row_highlight,  int,  4)

struct Settings {
    #define X(n, t, d) t n = d;
    SETTINGS(X)
    #undef X
};

Settings& settings();

// Functions for Java to access settings
char const* get_setting_name(int i);
int         get_setting_value(int i);
void        set_setting_value(int i, int v);
