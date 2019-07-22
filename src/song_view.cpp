#include "song_view.hpp"
#include "track_view.hpp"
#include "edit.hpp"
#include "player.hpp"
#include "foo.hpp"
#include <algorithm>


namespace {

int  m_song_scroll = 0;
int  m_block       = 0;

} // namespace


int get_selected_block() { return m_block; }

void draw_song_view() {

    Song& song = player::song();

    // mute buttons
    auto widths = calculate_column_widths({ BUTTON_BIG, gui::SEPARATOR_WIDTH, -1, -1, -1, -1, gui::SEPARATOR_WIDTH, SCROLLBAR_WIDTH });
    gui::padding({ widths[0], BUTTON_SMALL });
    gui::same_line();
    gui::separator();
    for (int c = 0; c < CHANNEL_COUNT; ++c) {
        gui::same_line();
        gui::min_item_size({ widths[c + 2], BUTTON_SMALL });
        bool  a = player::is_channel_active(c);
        float l = player::channel_level(c);
        if (gui::channel_button(a, l)) {
            player::set_channel_active(c, !a);
        }
    }
    gui::same_line();
    gui::separator();
    gui::padding({ BUTTON_SMALL, 0 });
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

        char str[4];
        snprintf(str, 3, "%02X", block_nr);
        gui::min_item_size({ widths[0], BUTTON_SMALL });
        if (gui::button(str, block_nr == m_block)) {
            m_block = block_nr;
            if (!player::is_playing()) player::block(m_block);
        }
        gui::same_line();
        gui::separator();
        if (block_nr >= song.table_length) {
            for (int c = 0; c < CHANNEL_COUNT; ++c) {
                gui::same_line();
                gui::min_item_size({ widths[c + 2], BUTTON_SMALL });
                gui::padding({});
            }
        }
        else {

            Song::Block& block = table[block_nr];
            for (int c = 0; c < CHANNEL_COUNT; ++c) {
                gui::same_line();
                sprint_track_id(str, block[c]);
                gui::min_item_size({ widths[c + 2], BUTTON_SMALL });
                if (gui::button(str)) {
                    enter_track_select(block[c]);
                }
                if (block[c] > 0 && gui::hold()) {
                    select_track(block[c]);
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


