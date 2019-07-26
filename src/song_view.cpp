#include "song_view.hpp"
#include "track_view.hpp"
#include "edit.hpp"
#include "player.hpp"
#include "foo.hpp"
#include <algorithm>


namespace {

int m_song_scroll;
int m_block;
int m_tempo_block;
} // namespace


int get_selected_block() { return m_block; }


void draw_tempo_select() {
    gui::min_item_size({ edit::screen_size().x, BUTTON_BIG });
    gui::text("SET TEMPO AND SWING");

    auto widths = calculate_column_widths({ -1, -1 });
    gui::min_item_size({ widths[0], BUTTON_BIG });
    if (gui::button("OK")) {
        edit::set_popup(nullptr);
    }
    gui::same_line();

    Song& song = player::song();
    Song::Block& block = song.table[m_tempo_block];

    gui::min_item_size({ widths[1], BUTTON_BIG });
    if (gui::button("CLEAR")) {
        edit::set_popup(nullptr);
        block.tempo = 0;
        block.swing = 0;
    }
    gui::separator();

    // tempo and swing
    widths = calculate_column_widths({ -12, -9 });
    gui::min_item_size({ widths[0], BUTTON_SMALL });
    gui::drag_int("TEMPO", "%X", block.tempo, 4, 15);
    gui::same_line();
    gui::min_item_size({ widths[1], BUTTON_SMALL });
    gui::drag_int("SWING", "%X", block.swing, 0, 8);
}


void draw_song_view() {

    Song& song = player::song();

    // mute buttons
    auto widths = calculate_column_widths({ -1, -1, -1, -1, BUTTON_BIG * 2 + gui::SEPARATOR_WIDTH * 3 + SCROLLBAR_WIDTH });
    gui::padding({ BUTTON_BIG, BUTTON_SMALL });
    gui::same_line();
    gui::separator();
    gui::padding({ BUTTON_BIG, BUTTON_SMALL });
    gui::same_line();
    gui::separator();
    for (int c = 0; c < CHANNEL_COUNT; ++c) {
        gui::same_line();
        gui::min_item_size({ widths[c], BUTTON_SMALL });
        bool  a = player::is_channel_active(c);
        float l = player::channel_level(c);
        if (gui::channel_button(a, l)) {
            player::set_channel_active(c, !a);
        }
    }
    gui::same_line();
    gui::separator();
    gui::next_line();
    gui::min_item_size({ edit::screen_size().x, 0 });
    gui::separator();

    // prepare scrollbar
    Vec c1 = gui::cursor();
    int free_space = edit::screen_size().y - gui::cursor().y -
            gui::SEPARATOR_WIDTH - BUTTON_BIG - gui::SEPARATOR_WIDTH - BUTTON_BAR;
    int page_length = free_space / BUTTON_SMALL;

    int max_scroll = std::max<int>(0, song.table_length - page_length);
    m_song_scroll = std::min(m_song_scroll, max_scroll);


    auto& table = song.table;

    int player_block = player::block();
    for (int i = 0; i < page_length; ++i) {
        int block_nr = m_song_scroll + i;
        if (block_nr == player_block) gui::button_theme(gui::BT_CURSOR);
        else gui::button_theme(gui::BT_NORMAL);

        char str[16];
        snprintf(str, 3, "%02X", block_nr);
        gui::min_item_size({ BUTTON_BIG, BUTTON_SMALL });

        if (block_nr >= song.table_length) gui::text(str);
        else if (gui::button(str, block_nr == m_block)) {
            m_block = block_nr;
            if (!player::is_playing()) player::block(m_block);
        }
        gui::same_line();
        gui::separator();


        if (block_nr >= song.table_length) {
            gui::padding({ BUTTON_BIG, BUTTON_SMALL });
            gui::same_line();
            gui::separator();

            for (int c = 0; c < CHANNEL_COUNT; ++c) {

                gui::same_line();
                gui::min_item_size({ widths[c], BUTTON_SMALL });
                gui::padding({});
            }
        }
        else {
            Song::Block& block = table[block_nr];
            str[0] = '\0';
            if (block.tempo > 0) sprintf(str, "%X%X", block.tempo, block.swing);
            gui::min_item_size({ BUTTON_BIG, BUTTON_SMALL });
            if (gui::button(str)) {
                m_tempo_block = block_nr;
                if (block.tempo == 0) block.tempo = 8;
                edit::set_popup(draw_tempo_select);
            }
            gui::same_line();
            gui::separator();

            for (int c = 0; c < CHANNEL_COUNT; ++c) {
                gui::same_line();
                sprint_track_id(str, block.tracks[c]);
                gui::min_item_size({ widths[c], BUTTON_SMALL });
                if (gui::button(str)) {
                    enter_track_select(block.tracks[c]);
                }
                if (block.tracks[c] > 0 && gui::hold()) {
                    select_track(block.tracks[c]);
                    edit::set_view(VIEW_TRACK);
                }
            }
        }

        gui::same_line();
        gui::separator();
        gui::next_line();

    }
    gui::button_theme(gui::BT_NORMAL);

    // song scrollbar
    Vec c2 = gui::cursor();
    gui::cursor({ edit::screen_size().x - SCROLLBAR_WIDTH, c1.y });
    gui::min_item_size({ SCROLLBAR_WIDTH, c2.y - c1.y });
    gui::drag_theme(gui::DT_SCROLLBAR);
    gui::vertical_drag_int(m_song_scroll, 0, max_scroll, page_length);
    gui::drag_theme(gui::DT_NORMAL);
    gui::cursor(c2);

    gui::min_item_size({ edit::screen_size().x, 0 });
    gui::separator();


    // buttons
    gui::min_item_size({ BUTTON_BIG, BUTTON_BIG });


    if (gui::button(gui::I_DELETE_ROW)) {
        if (m_block < song.table_length && song.table_length > 1) {
            table[m_block] = {};
            std::rotate(
                table.begin() + m_block,
                table.begin() + m_block + 1,
                table.begin() + song.table_length);
            --song.table_length;
            if (m_block == song.table_length) --m_block;
        }
    }
    gui::same_line();
    if (gui::button(gui::I_ADD_ROW_ABOVE)) {
        if (m_block <= song.table_length && song.table_length < MAX_SONG_LENGTH) {
            std::rotate(
                table.begin() + m_block,
                table.begin() + song.table_length,
                table.begin() + song.table_length + 1);
            ++song.table_length;
            ++m_block;
        }
    }
    gui::same_line();
    if (gui::button(gui::I_ADD_ROW_BELOW)) {
        if (m_block < song.table_length && song.table_length < MAX_SONG_LENGTH) {
            std::rotate(
                table.begin() + m_block + 1,
                table.begin() + song.table_length,
                table.begin() + song.table_length + 1);
            ++song.table_length;
        }
    }


    gui::min_item_size({ edit::screen_size().x, 0 });
    gui::separator();
}


