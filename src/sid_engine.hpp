#pragma once
#include <cmath>
#include <cstdint>


class SidEngine {
public:
    virtual int     chan_level(int c) const = 0;
    virtual uint8_t chan_reg(int c, int r) const = 0;
    virtual void    set_chan_reg(int c, int r, uint8_t value) = 0;

    virtual uint8_t global_reg(int r) const = 0;
    virtual void    set_global_reg(int r, uint8_t value) = 0;

    virtual void    mix(short* buffer, int length) = 0;

    void set_chan_freq(int c, float pitch) {
        uint16_t v = exp2f((pitch - 58) * (1.0f / 12)) * ((1 << 28) * (440 / 15872000.0f));
        set_chan_reg(c, 0, v & 0xff);
        set_chan_reg(c, 1, v >> 8);
    }

    static SidEngine& get_tinysid();
};
