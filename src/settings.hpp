#pragma once

struct Settings {
    int  track_row_highlight = 4;
    bool play_in_background  = false;
};

Settings& settings();

void load_settings();
void save_settings();
