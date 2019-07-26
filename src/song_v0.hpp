#pragma once
#include <array>

namespace v0 {

    enum {
        CHANNEL_COUNT         = 4,
        MAX_TRACK_LENGTH      = 32,
        TRACK_COUNT           = 21 * 12,
        INSTRUMENT_COUNT      = 48,
        EFFECT_COUNT          = INSTRUMENT_COUNT,
        MAX_INSTRUMENT_LENGTH = 16,
        MAX_FILTER_LENGTH     = 16,
        MAX_EFFECT_LENGTH     = 16,
        MAX_NAME_LENGTH       = 16,
        MAX_SONG_LENGTH       = 256,
    };


    struct Track {
        struct Row {
            uint8_t instrument;
            uint8_t effect;
            uint8_t note;
        };
        std::array<Row, MAX_TRACK_LENGTH> rows;
    };


    struct Filter {
        enum {
            T_LOW  = 1,
            T_BAND = 2,
            T_HIGH = 4,
        };

        enum {
            OP_INC,
            OP_SET,
            OP_DEC,
        };
        struct Row {
            uint8_t type      = T_LOW;
            uint8_t resonance = 15;
            uint8_t operation = OP_SET;
            uint8_t value     = 10;
        };

        uint8_t                            routing;
        uint8_t                            length;
        uint8_t                            loop;
        std::array<Row, MAX_FILTER_LENGTH> rows;
    };


    struct Instrument {
        enum {
            F_GATE  = 0x01,
            F_SYNC  = 0x02,
            F_RING  = 0x04,
            F_TRI   = 0x10,
            F_SAW   = 0x20,
            F_PULSE = 0x40,
            F_NOISE = 0x80,
        };
        enum {
            OP_INC,
            OP_SET,
        };
        struct Row {
            uint8_t flags     = F_GATE | F_PULSE;
            uint8_t operation = OP_SET;
            uint8_t value     = 16;
        };

        std::array<char, MAX_NAME_LENGTH>      name;
        std::array<uint8_t, 4>                 adsr         = { 0, 0, 12, 3 };
        uint8_t                                hard_restart = true;

        uint8_t                                length;
        uint8_t                                loop;
        std::array<Row, MAX_INSTRUMENT_LENGTH> rows;

        Filter                                 filter;
    };


    struct Effect {
        enum {
            OP_RELATIVE,
            OP_ABSOLUTE,
            OP_DETUNE,
        };
        struct Row {
            uint8_t operation = OP_RELATIVE;
            uint8_t value     = 0x30;
        };
        std::array<char, MAX_NAME_LENGTH>  name;
        uint8_t                            length;
        uint8_t                            loop;
        std::array<Row, MAX_EFFECT_LENGTH> rows;
    };


    struct Song {
        using Block = std::array<uint8_t, CHANNEL_COUNT>;

        std::array<char, 32>                     title;
        std::array<char, 32>                     author;

        uint8_t tempo; // 4 to F
        uint8_t swing; // 0 to 8
        uint8_t track_length;

        std::array<Track, TRACK_COUNT>           tracks;
        std::array<Instrument, INSTRUMENT_COUNT> instruments;
        std::array<Effect, EFFECT_COUNT>         effects;
        std::array<Block, MAX_SONG_LENGTH>       table;
        uint16_t                                 table_length;
    };

} // namespace
