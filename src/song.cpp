#include "song.hpp"
#include <cstring>


void init_song(Song& song) {
    //memset(&song, 0, sizeof(song));
    song = {};

    song.tempo = 8;
    song.track_length = 32;
    song.table_length = 1;
    song.table[0][0] = 1;


    // preset some instruments/effects

    enum {
        GLIDE_UP = 46,
        VIBRATO  = 47,
    };

    // lead
    {
        Instrument& i = song.instruments[0];
        strcpy(i.name.data(), "LEAD");
        i.adsr = { 0, 8, 12, 3 };
        i.rows[0] = { Instrument::F_GATE | Instrument::F_NOISE, Instrument::OP_SET, 7 };
        i.rows[1] = { Instrument::F_GATE | Instrument::F_PULSE, Instrument::OP_INC, 3 };
        i.length = 2;
        i.loop = 1;
        i.hard_restart = true;
        Effect& e = song.effects[0];
        strcpy(e.name.data(), "LEAD");
        e.rows[0] = { Effect::OP_ABSOLUTE, 70 };
        e.rows[1] = { Effect::OP_RELATIVE, 0x30 };
        e.length = 2;
        e.loop = 1;
    }


    // glide up
    {
        Effect& e = song.effects[GLIDE_UP];
        strcpy(e.name.data(), "GLIDE UP");
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
        strcpy(e.name.data(), "VIBRATO");
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
    FILE* file = fopen(name, "rb");
    if (!file) return false;
    fread(song.title.data(), sizeof(char), song.title.size(), file);
    fread(song.author.data(), sizeof(char), song.author.size(), file);
    fread(&song.tempo, sizeof(uint8_t), 1, file);
    fread(&song.swing, sizeof(uint8_t), 1, file);
    fread(&song.track_length, sizeof(uint8_t), 1, file);
    fread(song.tracks.data(), sizeof(Track), song.tracks.size(), file);
    fread(song.instruments.data(), sizeof(Instrument), song.instruments.size(), file);
    fread(song.effects.data(), sizeof(Effect), song.effects.size(), file);
    fread(&song.table_length , sizeof(uint16_t), 1, file);
    song.table = {};
    fread(song.table.data(), sizeof(Song::Block), song.table_length, file);
    fclose(file);
    return true;
}


bool save_song(Song const& song, char const* name) {
    FILE* file = fopen(name, "wb");
    if (!file) return false;
    fwrite(song.title.data(), sizeof(char), song.title.size(), file);
    fwrite(song.author.data(), sizeof(char), song.author.size(), file);
    fwrite(&song.tempo, sizeof(uint8_t), 1, file);
    fwrite(&song.swing, sizeof(uint8_t), 1, file);
    fwrite(&song.track_length, sizeof(uint8_t), 1, file);
    fwrite(song.tracks.data(), sizeof(Track), song.tracks.size(), file);
    fwrite(song.instruments.data(), sizeof(Instrument), song.instruments.size(), file);
    fwrite(song.effects.data(), sizeof(Effect), song.effects.size(), file);
    fwrite(&song.table_length, sizeof(uint16_t), 1, file);
    fwrite(song.table.data(), sizeof(Song::Block), song.table_length, file);
    fclose(file);
    return true;
}
