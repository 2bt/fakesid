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
    int y2 = edit::screen_size().y  - BUTTON_BAR - gui::SEPARATOR_WIDTH;
    auto widths  = calculate_column_widths(std::vector<int>(12, -1));
    auto heights = calculate_column_widths(std::vector<int>(8, -1), y2 - y1);


    int prev_note = m_jam_note;

    gui::button_theme(gui::BT_JAM);
    for (int y = 0; y < (int) heights.size(); ++y) {
        for (int x = 0; x < (int) widths.size(); ++x) {
            if (x) gui::same_line();

            gui::min_item_size({ widths[x], heights[y] });
            gui::Box box = gui::padding({});
            Vec c = gui::cursor();
            gui::cursor(box.pos);

            int note = x + (7 - y) * 12 + 1;
            int index = 1;
            if ((1 << x) & 0b010101001010) index = 0;
            if (gui::touch().pressed && box.contains(gui::touch().pos)) {
                m_jam_note = note;
                index = 2;
            }

            gui::dumb_button(index);
            gui::cursor(c);
        }
    }
    gui::button_theme(gui::BT_NORMAL);

    if (gui::touch().just_released()) {
        m_jam_note = 0;
        player::jam({ 0, 0, 255 });
    }
    else if (m_jam_note && m_jam_note != prev_note) {
        Track::Row row = { 0, 0, m_jam_note };

        if (gui::touch().just_pressed()) {
            row.instrument = selected_instrument();
            row.effect     = selected_effect();
        }
        player::jam(row);
    }

    gui::separator();
}
