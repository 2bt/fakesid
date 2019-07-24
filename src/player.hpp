#pragma once
#include "song.hpp"


enum {
    MIXRATE               = 44100,
    FRAMES_PER_SECOND     = 50,
    SAMPLES_PER_FRAME     = MIXRATE / FRAMES_PER_SECOND,
};


namespace player {
    void  fill_buffer(short* buffer, int length);
    void  set_playing(bool p);
    bool  is_playing();
    int   row();
    int   block();
    void  block(int b);
    bool  block_loop();
    void  block_loop(bool b);
    float channel_level(int c);
    bool  is_channel_active(int c);
    void  set_channel_active(int c, bool a);
    void  jam(Track::Row const& row);
    Song& song();
}
