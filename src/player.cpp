#include "player.hpp"
#include <cmath>


namespace player {
namespace {


constexpr Filter     null_filter     = {};
constexpr Instrument null_instrument = {};
constexpr Effect     null_effect     = {};

constexpr std::array<int, 16> attack_speeds = {
    168867, 47495, 24124, 15998, 10200, 6908, 5692, 4855,
    3877, 1555, 777, 486, 389, 129, 77, 48,
};

constexpr std::array<int, 16> release_speeds = {
    42660, 15468, 7857, 5210, 3322, 2250, 1853, 1581,
    1262, 506, 253, 158, 126, 42, 25, 15,
};


enum State { RELEASE, ATTACK, DECAY, SUSTAIN };


struct Channel {
    bool              active = true;

    int               note;
    bool              gate;
    Instrument const* inst = &null_instrument;
    int               inst_row;
    Effect const*     effect = &null_effect;
    int               effect_row;
    int               pulsewidth_acc;

    State    state;
    int      adsr[4];
    int      flags;
    uint32_t next_pulsewidth;
    uint32_t pulsewidth;
    uint32_t freq;


    // internal things
    int      level;
    uint32_t phase;
    uint32_t noise_phase;
    uint32_t shift = 0x7ffff8;
    int      noise;
    bool     filter;
};


struct {
    Filter const* filter = &null_filter;
    int           row;
    int           freq_acc;

    uint8_t       type;
    float         resonance;
    float         freq;

    float         high;
    float         band;
    float         low;

} m_filter;


Song m_song;
bool m_is_playing;
int  m_sample;
int  m_frame;
int  m_row;
int  m_block;
bool m_block_loop;
std::array<Channel, CHANNEL_COUNT> m_channels;
Track::Row m_jam_row;

void apply_track_row(Channel& chan, Track::Row const& row) {
    // instrument
    if (row.instrument > 0) {
        chan.inst = &m_song.instruments[row.instrument - 1];
        Instrument const& inst = *chan.inst;
        chan.adsr[0] = attack_speeds[inst.adsr[0]];
        chan.adsr[1] = release_speeds[inst.adsr[1]];
        chan.adsr[2] = inst.adsr[2] * 0x111111;
        chan.adsr[3] = release_speeds[inst.adsr[3]];
        chan.inst_row = 0;
        chan.gate = true;

        // cause envelop restart
        // XXX: do we want that?
        //chan.state = RELEASE;

        // filter
        if (inst.filter.length > 0) {
            m_filter.filter = &inst.filter;
            m_filter.row = 0;
            for (int i = 0; i < CHANNEL_COUNT; ++i) {
                m_channels[i].filter = (inst.filter.routing & (1 << i)) != 0;
            }
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
        chan.freq = exp2f((chan.note - 58) / 12.0f) * (1 << 28) * 440 / MIXRATE;
    }
}


void tick() {
    // jam
    apply_track_row(m_channels.back(), m_jam_row);
    m_jam_row = {};

    // row_update
    if (m_is_playing && m_frame == 0) {
        int block_nr = m_block;
        if (block_nr >= m_song.table_length) block_nr = 0;
        Song::Block const& block = m_song.table[block_nr];

        for (int c = 0; c < CHANNEL_COUNT; ++c) {
            Channel& chan = m_channels[c];
            int track_nr = block[c];
            if (track_nr == 0) continue;
            Track const& track = m_song.tracks[track_nr - 1];
            apply_track_row(chan, track.rows[m_row]);
        }
    }

    // frame update
    for (int c = 0; c < CHANNEL_COUNT; ++c) {
        Channel& chan = m_channels[c];

        // instrument
        Instrument const& inst = *chan.inst;
        if (inst.length > 0) {
            if (chan.inst_row >= inst.length) {
                chan.inst_row = std::min<int>(inst.loop, inst.length - 1);
            }
            Instrument::Row row = inst.rows[chan.inst_row++];
            chan.flags = row.flags;
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
            chan.next_pulsewidth = p * 0x7cccc;
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
            case Effect::OP_RELATIVE:
                n = chan.note - 58 + row.value - 0x30;
                break;
            case Effect::OP_ABSOLUTE:
                n = 1         - 58 + row.value;
                break;
            case Effect::OP_DETUNE:
                n = chan.note - 58 + (row.value - 0x30) * 0.25;
                break;
            default:
                break;
            }
            chan.freq = exp2f(n / 12) * (1 << 28) * 440 / MIXRATE;
        }
    }

    // filter
    Filter const& filter = *m_filter.filter;
    if (filter.length > 0) {
        if (m_filter.row >= filter.length) {
            m_filter.row = std::min<int>(filter.loop, filter.length - 1);
        }
        Filter::Row row = filter.rows[m_filter.row++];
        m_filter.type = row.type;
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

        m_filter.freq = m_filter.freq_acc * (21.5332031 / MIXRATE);
        m_filter.resonance = 1.2 - 0.04 * row.resonance;
    }


    if (!m_is_playing) return;


    int frames_per_row = m_song.tempo;
    if (m_row % 2 == 0) frames_per_row += m_song.swing;

    // hard restart
    // look two frames into the future
    if (m_frame == frames_per_row - 2) {
        int block_nr = m_block;
        int row_nr   = m_row + 1;
        if (row_nr >= m_song.track_length) {
            row_nr = 0;
            if (!m_block_loop && ++block_nr >= m_song.table_length) {
                block_nr = 0;
            }
        }
        if (block_nr >= m_song.table_length) block_nr = 0;
        Song::Block const& block = m_song.table[block_nr];

        for (int c = 0; c < CHANNEL_COUNT; ++c) {
            Channel& chan = m_channels[c];
            int track_nr = block[c];
            if (track_nr == 0) continue;
            Track const& track = m_song.tracks[track_nr - 1];
            Track::Row const& row = track.rows[row_nr];

            if (row.instrument > 0) {
                Instrument const& inst = m_song.instruments[row.instrument - 1];
                if (inst.hard_restart) {
                    chan.gate = false;
                    chan.adsr[2] = 0;
                    chan.adsr[3] = release_speeds[0];
                }
            }
        }
    }


    // advance
    if (++m_frame >= frames_per_row) {
        m_frame = 0;
        if (++m_row >= m_song.track_length) {
            m_row = 0;
            if (!m_block_loop && ++m_block >= m_song.table_length) {
                m_block = 0;
            }
        }
    }
}


void mix(short* buffer, int length) {
    for (int i = 0; i < length; ++i) {

        int out[2] = {};

        for (int c = 0; c < CHANNEL_COUNT; ++c) {
            Channel& chan = m_channels[c];
            Channel& prev_chan = m_channels[c == 0 ? CHANNEL_COUNT - 1 : c - 1];

            // osc
            chan.phase += chan.freq;
            chan.phase &= 0xfffffff;

            // sync
            if (prev_chan.phase < prev_chan.freq) {
                if (chan.flags & Instrument::F_SYNC) {
                    chan.phase = prev_chan.phase * chan.freq / prev_chan.freq;
                }
            }

            if (!chan.active) continue;

            // envelope
            bool gate = chan.gate && (chan.flags & Instrument::F_GATE);
            if (gate && chan.state == RELEASE) chan.state = ATTACK;
            if (!gate) chan.state = RELEASE;

            switch (chan.state) {
            case ATTACK:
                chan.level += chan.adsr[0];
                if (chan.level >= 0xffffff) {
                    chan.level = 0xffffff;
                    chan.state = DECAY;
                }
                break;
            case DECAY:
                chan.level -= chan.adsr[1];
                if (chan.level <= chan.adsr[2]) {
                    chan.level = chan.adsr[2];
                    chan.state = SUSTAIN;
                }
                break;
            case SUSTAIN:
                if (chan.level != chan.adsr[2]) chan.state = ATTACK;
                break;
            case RELEASE:
                chan.level -= chan.adsr[3];
                if (chan.level < 0) chan.level = 0;
                break;
            }


            // smooth pulsewith change
            if (chan.phase < chan.freq) {
                chan.pulsewidth = chan.next_pulsewidth;
            }

            // waveforms
            uint8_t tri   = ((chan.phase < 0x8000000 ? chan.phase : ~chan.phase) >> 19) & 0xff;
            uint8_t saw   = (chan.phase >> 20) & 0xff;
            uint8_t pulse = ((chan.phase > chan.pulsewidth) - 1) & 0xff;
            if (chan.noise_phase != chan.phase >> 23) {
                chan.noise_phase = chan.phase >> 23;
                uint32_t s = chan.shift;
                chan.shift = s = (s << 1) | (((s >> 22) ^ (s >> 17)) & 1);
                chan.noise = ((s & 0x400000) >> 11) |
                             ((s & 0x100000) >> 10) |
                             ((s & 0x010000) >>  7) |
                             ((s & 0x002000) >>  5) |
                             ((s & 0x000800) >>  4) |
                             ((s & 0x000080) >>  1) |
                             ((s & 0x000010) <<  1) |
                             ((s & 0x000004) <<  2);
            }
            uint8_t noise = chan.noise;

            // ringmod
            if (chan.flags & Instrument::F_RING && prev_chan.phase < 0x8000000) {
                tri = ~tri;
            }

            int v = 0xff;
            if (chan.flags & Instrument::F_TRI)   v &= tri;
            if (chan.flags & Instrument::F_SAW)   v &= saw;
            if (chan.flags & Instrument::F_PULSE) v &= pulse;
            if (chan.flags & Instrument::F_NOISE) v &= noise;

            v = ((v - 0x80) * chan.level) >> 18;

            out[chan.filter] += v;
        }


        // filter
        m_filter.high = out[1] - m_filter.band * m_filter.resonance - m_filter.low;
        m_filter.band += m_filter.freq * m_filter.high;
        m_filter.low  += m_filter.freq * m_filter.band;
        int f = 0;
        if (m_filter.type & FILTER_LOW)  f += m_filter.low;
        if (m_filter.type & FILTER_BAND) f += m_filter.band;
        if (m_filter.type & FILTER_HIGH) f += m_filter.high;

        int sample = out[0] + f;
        buffer[i] = std::max(-32768, std::min<int>(sample, 32767));
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
        mix(buffer, l);
        buffer += l;
    }
}


void reset() {
    m_sample = 0;
    m_frame = 0;
    m_row = 0;
    m_block = 0;
    m_filter = {};
    for (Channel& chan : m_channels) {
        bool a = chan.active;
        chan = {};
        chan.active = a;
    }
}


void set_playing(bool p) {
    m_is_playing = p;
    if (!m_is_playing) {
        m_filter = {};
        for (Channel& chan : m_channels) {
            chan.gate = false;
            chan.adsr[2] = 0;
            chan.adsr[3] = release_speeds[0];
        }
    }
}

int   row() { return m_row; }
int   block() { return m_block; }
void  block(int b) {
    m_block = std::min(b, (int) m_song.table_length - 1);
}
bool  block_loop() { return m_block_loop; }
void  block_loop(bool b) { m_block_loop = b; }
bool  is_channel_active(int c) { return m_channels[c].active; }
void  set_channel_active(int c, bool a) { m_channels[c].active = a; }
void  jam(Track::Row const& row) { m_jam_row = row; }
Song& song() { return m_song; }
bool  is_playing() { return m_is_playing; }


} // namespace;
