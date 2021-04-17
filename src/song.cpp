#include "song.hpp"
#include "song_v0.hpp"
#include <cstring>
#include <cstdio>


namespace  {


size_t fread(uint8_t& b, FILE *stream) {
    return fread(&b, sizeof(uint8_t), 1, stream);
}

size_t fread(uint16_t& b, FILE *stream) {
    return fread(&b, sizeof(uint16_t), 1, stream);
}

template<class T>
size_t fread(T& t, FILE *stream) {
    return fread(t.data(), sizeof(t[0]), t.size(), stream);
}

size_t fwrite(uint8_t const& b, FILE *stream) {
    return fwrite(&b, sizeof(uint8_t), 1, stream);
}

size_t fwrite(uint16_t const& b, FILE *stream) {
    return fwrite(&b, sizeof(uint16_t), 1, stream);
}

template<class T>
size_t fwrite(T const& t, FILE *stream) {
    return fwrite(t.data(), sizeof(t[0]), t.size(), stream);
}


void convert_song(Song& song, v0::Song const& song_v0) {
    song.title        = song_v0.title;
    song.author       = song_v0.author;
    song.table_length = song_v0.table_length;
    song.instruments  = *(decltype(&song.instruments)) &song_v0.instruments;
    song.effects      = *(decltype(&song.effects)) &song_v0.effects;
    for (int i = 0; i < (int) song.tracks.size(); ++i) {
        song.tracks[i].length = song_v0.track_length;
        song.tracks[i].rows   = *(decltype(&song.tracks[i].rows)) &song_v0.tracks[i].rows;
    }
    for (int i = 0; i < song.table_length; ++i) {
        song.table[i].tracks = song_v0.table[i];
        if (i == 0) {
            song.table[i].tempo = song_v0.tempo;
            song.table[i].swing = song_v0.swing;
        }
        else {
            song.table[i].tempo = 0;
            song.table[i].swing = 0;
        }
    }
}


} // namespace


namespace v0 {
    bool load_song(Song& song, char const* name) {
        FILE* file = fopen(name, "rb");
        if (!file) return false;
        fread(song.title, file);
        fread(song.author, file);
        fread(song.tempo, file);
        fread(song.swing, file);
        fread(song.track_length, file);
        fread(song.tracks, file);
        fread(song.instruments, file);
        fread(song.effects, file);
        fread(song.table_length, file);
        song.table = {};
        fread(song.table.data(), sizeof(Song::Block), song.table_length, file);
        fclose(file);
        return true;
    }

} // namespace





void init_song(Song& song) {
    song = {};

    song.table[0].tempo = 8;
    song.table_length = 1;
    song.table[0].tracks[0] = 1;
    song.tracks[0].rows[0] = { 1, 1, 49 };
    song.tracks[0].rows[1] = { 0, 0, 255 };


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

    std::array<char, 4> magic;
    fread(magic, file);
    if (magic != std::array<char, 4>{ 'S', 'N', 'G', '\x17' }) {
        fclose(file);
        // try loading song version 0
        v0::Song song_v0;
        if (!v0::load_song(song_v0, name)) return false;
        convert_song(song, song_v0);
        return true;
    }

    uint8_t version;
    fread(version, file);
    if (version != 1) {
        fclose(file);
        return false;
    }

    fread(song.title, file);
    fread(song.author, file);
    fread(song.tracks, file);
    fread(song.instruments, file);
    fread(song.effects, file);
    fread(song.table_length, file);
    song.table = {};
    fread(song.table.data(), sizeof(Song::Block), song.table_length, file);
    fclose(file);
    return true;
}

bool save_song(Song const& song, char const* name) {
    FILE* file = fopen(name, "wb");
    if (!file) return false;
    fwrite("SNG\x17\x01", sizeof(char), 5, file);
    fwrite(song.title, file);
    fwrite(song.author, file);
    fwrite(song.tracks, file);
    fwrite(song.instruments, file);
    fwrite(song.effects, file);
    fwrite(song.table_length, file);
    fwrite(song.table.data(), sizeof(Song::Block), song.table_length, file);
    fclose(file);
    return true;
}
