#include "gui.hpp"
#include "edit.hpp"
#include "track_view.hpp"
#include "player.hpp"

namespace {

uint8_t m_jam_note;

} // namespace


void draw_jam_view() {

    // cache
    draw_instrument_cache();
    gui::separator();
    draw_effect_cache();
    gui::separator();

    int y1 = gui::cursor().y;
    int y2 = edit::screen_size().y  - BUTTON_BIG - gui::SEPARATOR_WIDTH;
    auto widths  = calculate_column_widths(std::vector<int>(12, -1));
    auto heights = calculate_column_widths(std::vector<int>(8, -1), y2 - y1);


//    input::Touch const* touch = nullptr;
//    for (int i = 0; i < 2; ++i) {
//        input::Touch const& t = input::touch(i);
//        if (t.state == input::Touch::JUST_PRESSED) touch = &t;
//    }

/*
    int prev_note = m_jam_note;

    for (int y = 0; y < (int) heights.size(); ++y) {

        for (int x = 0; x < (int) widths.size(); ++x) {
            if (x) gui::same_line();

            gui::min_item_size({ widths[x], heights[y] });

            Color color = gui::color::make(0x222222);
            if ((1 << x) & 0b010101001010) color = gui::color::make(0x111111);
            gfx::color(color);

            gui::Box box = gui::padding({ widths[x], heights[y] });
            gfx::rectangle(box.pos, box.size, 0);


            int note = x + (7 - y) * 12 + 1;
            if (touch && box.contains(touch->pos)) {
                m_jam_touch = touch;
            }
            if (m_jam_touch && box.contains(m_jam_touch->pos)) {
                m_jam_note = note;
                gfx::color(gui::color::make(0x55a049));
                gfx::rectangle(box.pos, box.size, 2);
            }
        }
    }

    if (m_jam_touch && m_jam_touch->state == input::Touch::JUST_RELEASED) {
        m_jam_touch = nullptr;
        m_jam_note = 0;
        player::jam({ 0, 0, 255 });
    }
    else if (m_jam_note && m_jam_note != prev_note) {
        Track::Row row = { 0, 0, m_jam_note };
        if (touch) {
            row.instrument = selected_instrument();
            row.effect     = selected_effect();
        }
        player::jam(row);
    }
*/
    gui::separator();
}
