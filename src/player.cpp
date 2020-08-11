#include "player.hpp"
#include "sid_engine.hpp"


namespace player {
namespace {



constexpr Filter     null_filter     = {};
const     Instrument null_instrument = {};
constexpr Effect     null_effect     = {};


struct Channel {
    Channel(int id) : id(id) {}

    int const         id;
    bool              active = true;

    int               note;
    bool              gate;
    Instrument const* inst = &null_instrument;
    int               inst_row;
    Effect const*     effect = &null_effect;
    int               effect_row;
    int               pulsewidth_acc;
};


struct {
    Filter const* filter = &null_filter;
    int           row;
    int           freq_acc;

} m_filter;


SidEngine& m_engine = SidEngine::get_tinysid();

Song    m_song;
bool    m_is_playing;
int     m_sample;
int     m_frame;
int     m_row;
int     m_block;
bool    m_block_loop;
uint8_t m_tempo;
uint8_t m_swing;

std::array<Channel, CHANNEL_COUNT> m_channels = {
    Channel{0},
    Channel{1},
    Channel{2},
    Channel{3}
};
Track::Row m_jam_row;

void apply_track_row(Channel& chan, Track::Row const& row) {
    // instrument
    if (row.instrument > 0) {
        chan.inst = &m_song.instruments[row.instrument - 1];
        Instrument const& inst = *chan.inst;

        if (chan.active) {
            m_engine.set_chan_reg(chan.id, 5, (inst.adsr[0] << 4) | inst.adsr[1]);
            m_engine.set_chan_reg(chan.id, 6, (inst.adsr[2] << 4) | inst.adsr[3]);
        }
        else {
            m_engine.set_chan_reg(chan.id, 5, 0x00);
            m_engine.set_chan_reg(chan.id, 6, 0x00);
        }

        chan.inst_row = 0;
        chan.gate = true;

        // filter
        if (inst.filter.length > 0) {
            m_filter.filter = &inst.filter;
            m_filter.row = 0;
            m_engine.set_global_reg(2, (m_engine.global_reg(2) & 0xf0) | (inst.filter.routing & 0xf));
        }
    }

    // effect
    if (row.effect > 0) {
        chan.effect = &m_song.effects[row.effect - 1];
        chan.effect_row = 0;
    }

    // note
    if (row.note == 255) {
        chan.gate = false;
    }
    else if (row.note > 0) {
        chan.note = row.note;
        m_engine.set_chan_freq(chan.id, chan.note);
    }
}


void tick() {
    // jam
    apply_track_row(m_channels.back(), m_jam_row);
    m_jam_row = {};

    int block_nr = m_block;
    if (block_nr >= m_song.table_length) {
        m_block = block_nr = 0;
    }
    Song::Block const& block = m_song.table[block_nr];

    // row_update
    if (m_is_playing && m_frame == 0) {

        for (Channel& chan : m_channels) {
            int track_nr = block.tracks[chan.id];
            if (track_nr == 0) continue;
            Track const& track = m_song.tracks[track_nr - 1];
            apply_track_row(chan, track.rows[m_row]);
        }
    }

    // frame update
    for (Channel& chan : m_channels) {
        // instrument
        Instrument const& inst = *chan.inst;
        if (inst.length > 0) {
            if (chan.inst_row >= inst.length) {
                chan.inst_row = std::min<int>(inst.loop, inst.length - 1);
            }
            Instrument::Row row = inst.rows[chan.inst_row++];
            uint8_t flags = row.flags;
            if (!chan.gate || !chan.active) flags &= ~1; // clear gate bit
            m_engine.set_chan_reg(chan.id, 4, flags);

            switch (row.operation) {
            case Instrument::OP_SET:
                chan.pulsewidth_acc = row.value * 0x10;
                break;
            case Instrument::OP_INC:
                chan.pulsewidth_acc += row.value;
                chan.pulsewidth_acc &= 0x1ff;
                break;
            default:
                break;
            }
            uint32_t p = chan.pulsewidth_acc > 0xff ? chan.pulsewidth_acc : ~chan.pulsewidth_acc & 0x1ff;
            uint16_t pw = p * 0x7ccff >> 16;
            m_engine.set_chan_reg(chan.id, 2, pw & 0xff);
            m_engine.set_chan_reg(chan.id, 3, (pw >> 8) & 0x0f);
        }

        // effect
        Effect const& effect = *chan.effect;
        if (effect.length > 0) {
            if (chan.effect_row >= effect.length) {
                chan.effect_row = std::min<int>(effect.loop, effect.length - 1);
            }
            Effect::Row row = effect.rows[chan.effect_row++];
            float n = 0;
            switch (row.operation) {
            case Effect::OP_RELATIVE: n = chan.note + row.value - 0x30; break;
            case Effect::OP_ABSOLUTE: n = 1         + row.value; break;
            case Effect::OP_DETUNE:   n = chan.note + (row.value - 0x30) * 0.25; break;
            default: break;
            }
            m_engine.set_chan_freq(chan.id, n);
        }
    }

    // filter
    Filter const& filter = *m_filter.filter;
    if (filter.length > 0) {
        if (m_filter.row >= filter.length) {
            m_filter.row = std::min<int>(filter.loop, filter.length - 1);
        }
        Filter::Row row = filter.rows[m_filter.row++];
        switch (row.operation) {
        case Filter::OP_SET:
            m_filter.freq_acc = 1 + row.value * 66;
            break;
        case Filter::OP_DEC:
            m_filter.freq_acc = std::max(0, m_filter.freq_acc - row.value * 4);
            break;
        case Filter::OP_INC:
            m_filter.freq_acc = std::min(0x7ff, m_filter.freq_acc + row.value * 4);
            break;
        default:
            break;
        }

        m_engine.set_global_reg(0, m_filter.freq_acc & 0x7);
        m_engine.set_global_reg(1, m_filter.freq_acc >> 3);
        m_engine.set_global_reg(2, (m_engine.global_reg(2) & 0xf) | (row.resonance << 4));
        m_engine.set_global_reg(3, (row.type << 4) | 0xf);
    }


    if (!m_is_playing) return;

    if (block.tempo > 0) {
        m_tempo = block.tempo;
        m_swing = block.swing;
    }
    int frames_per_row = m_tempo + (m_row % 2 == 0 ? m_swing : 0);

    // hard restart
    // look two frames into the future
    if (m_frame == frames_per_row - 2) {
        int block_nr = m_block;
        int row_nr   = m_row + 1;
        if (row_nr >= m_song.block_length(block_nr)) {
            row_nr = 0;
            if (!m_block_loop && ++block_nr >= m_song.table_length) {
                block_nr = 0;
            }
        }
        if (block_nr >= m_song.table_length) block_nr = 0;
        Song::Block const& block = m_song.table[block_nr];

        for (Channel& chan : m_channels) {
            int track_nr = block.tracks[chan.id];
            if (track_nr == 0) continue;
            Track const& track = m_song.tracks[track_nr - 1];
            Track::Row const& row = track.rows[row_nr];

            if (row.instrument > 0) {
                Instrument const& inst = m_song.instruments[row.instrument - 1];
                if (inst.hard_restart) {
                    chan.gate = false;
                    m_engine.set_chan_reg(chan.id, 4, m_engine.chan_reg(chan.id, + 4) & ~1);
                    m_engine.set_chan_reg(chan.id, 6, 0x00);
                }
            }
        }
    }


    // advance
    if (++m_frame >= frames_per_row) {
        m_frame = 0;
        if (++m_row >= m_song.block_length(m_block)) {
            m_row = 0;
            if (!m_block_loop && ++m_block >= m_song.table_length) {
                m_block = 0;
            }
        }
    }
}


} // namespace


void fill_buffer(short* buffer, int length) {
    while (length > 0) {
        if (m_sample == 0) tick();
        int l = std::min(SAMPLES_PER_FRAME - m_sample, length);
        m_sample += l;
        if (m_sample == SAMPLES_PER_FRAME) m_sample = 0;
        length -= l;
        m_engine.mix(buffer, l);
        buffer += l;
    }
}

void set_playing(bool p) {
    if (p) {
        m_song.get_block_tempo_and_swing(m_block, m_tempo, m_swing);
        m_is_playing = true;
    }
    else {
        m_is_playing = false;
        m_filter = {};
        for (Channel& chan : m_channels) {
            m_engine.set_chan_reg(chan.id, 5, 0x00);
            m_engine.set_chan_reg(chan.id, 6, 0x00);
        }
    }
}

int   row() { return m_row; }
int   block() { return m_block; }
void  block(int b) {
    m_block = std::max(0, std::min(b, (int) m_song.table_length - 1));
    m_song.get_block_tempo_and_swing(m_block, m_tempo, m_swing);
    m_sample = 0;
    m_frame = 0;
    m_row = 0;
    m_filter = {};
}
bool  block_loop() { return m_block_loop; }
void  block_loop(bool b) { m_block_loop = b; }
float channel_level(int c) { return m_engine.chan_level(c) / float(0xffffff); }
bool  is_channel_active(int c) { return m_channels[c].active; }
void  set_channel_active(int c, bool a) { m_channels[c].active = a; }
void  jam(Track::Row const& row) { m_jam_row = row; }
Song& song() { return m_song; }
bool  is_playing() { return m_is_playing; }

} // namespace player
