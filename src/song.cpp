#include "song.hpp"
#include <cstring>
//#include <SDL.h>


void init_song(Song& song) {
    //memset(&song, 0, sizeof(song));
    song = {};

    song.tempo = 8;
    song.track_length = 32;
    song.table_length = 1;

    // preset some instruments/effects

    enum {
        GLIDE_UP = 46,
        VIBRATO  = 47,
    };

    // lead
    {
        Instrument& i = song.instruments[0];
        strcpy(i.name.data(), "lead");
        i.adsr = { 0, 8, 12, 3 };
        i.rows[0] = { Instrument::F_GATE | Instrument::F_PULSE, Instrument::OP_SET, 7 };
        i.rows[1] = { Instrument::F_GATE | Instrument::F_PULSE, Instrument::OP_INC, 3 };
        i.length = 2;
        i.loop = 1;
        i.hard_restart = true;
    }


    // glide up
    {
        Effect& e = song.effects[GLIDE_UP];
        strcpy(e.name.data(), "glide up");
        e.rows[0] = { Effect::OP_DETUNE, 0x2c };
        e.rows[1] = { Effect::OP_DETUNE, 0x2d };
        e.rows[2] = { Effect::OP_DETUNE, 0x2e };
        e.rows[3] = { Effect::OP_DETUNE, 0x2f };
        e.rows[4] = { Effect::OP_DETUNE, 0x30 };
        e.length = 5;
        e.loop = 4;
    }

    // vibrato
    {
        Effect& e = song.effects[VIBRATO];
        strcpy(e.name.data(), "vibrato");
        e.rows[0] = { Effect::OP_DETUNE, 0x30 };
        e.rows[1] = { Effect::OP_DETUNE, 0x31 };
        e.rows[2] = { Effect::OP_DETUNE, 0x32 };
        e.rows[3] = { Effect::OP_DETUNE, 0x32 };
        e.rows[4] = { Effect::OP_DETUNE, 0x31 };
        e.rows[5] = { Effect::OP_DETUNE, 0x30 };
        e.rows[6] = { Effect::OP_DETUNE, 0x2f };
        e.rows[7] = { Effect::OP_DETUNE, 0x2e };
        e.rows[8] = { Effect::OP_DETUNE, 0x2e };
        e.rows[9] = { Effect::OP_DETUNE, 0x2f };
        e.length = 10;
        e.loop = 0;
    }
}


bool load_song(Song& song, char const* name) {
//    SDL_RWops* file = SDL_RWFromFile(name, "rb");
//    if (!file) return false;
//    SDL_RWread(file, song.title.data(), sizeof(char), song.title.size());
//    SDL_RWread(file, song.author.data(), sizeof(char), song.author.size());
//    song.tempo = SDL_ReadU8(file);
//    song.swing = SDL_ReadU8(file);
//    song.track_length = SDL_ReadU8(file);
//    SDL_RWread(file, song.tracks.data(), sizeof(Track), song.tracks.size());
//    SDL_RWread(file, song.instruments.data(), sizeof(Instrument), song.instruments.size());
//    SDL_RWread(file, song.effects.data(), sizeof(Effect), song.effects.size());
//    song.table_length = SDL_ReadLE16(file);
//    song.table = {};
//    SDL_RWread(file, song.table.data(), sizeof(Song::Block), song.table_length);
//    SDL_RWclose(file);
    return true;
}


bool save_song(Song const& song, char const* name) {
//    SDL_RWops* file = SDL_RWFromFile(name, "wb");
//    if (!file) return false;
//    SDL_RWwrite(file, song.title.data(), sizeof(char), song.title.size());
//    SDL_RWwrite(file, song.author.data(), sizeof(char), song.author.size());
//    SDL_WriteU8(file, song.tempo);
//    SDL_WriteU8(file, song.swing);
//    SDL_WriteU8(file, song.track_length);
//    SDL_RWwrite(file, song.tracks.data(), sizeof(Track), song.tracks.size());
//    SDL_RWwrite(file, song.instruments.data(), sizeof(Instrument), song.instruments.size());
//    SDL_RWwrite(file, song.effects.data(), sizeof(Effect), song.effects.size());
//    SDL_WriteLE16(file, song.table_length);
//    SDL_RWwrite(file, song.table.data(), sizeof(Song::Block), song.table_length);
//    SDL_RWclose(file);
    return true;
}
