#include "track_view.hpp"
#include "project_view.hpp"
#include "edit.hpp"
#include "settings.hpp"
#include "player.hpp"
#include <algorithm>


namespace {


struct Cache {
    Cache() {
        for (int i = 0; i < SIZE; ++i) {
            entries[i].age  = i;
            entries[i].data = i + 1;
        }
    }
    enum { SIZE = 12 };
    struct Entry {
        uint32_t age;
        int      data;
        bool operator<(Entry const& rhs) const {
            return data < rhs.data;
        }
    };
    std::array<Entry, SIZE> entries;
    void insert(int data) {
        Entry* f = &entries[0];
        for (Entry& e : entries) {
            if (e.data == data) e.age = -1;
            else ++e.age;
            if (e.age > f->age) {
                f = &e;
            }
        }
        f->data = data;
        f->age = 0;
        std::sort(entries.begin(), entries.end());
    }
};


void draw_cache(Cache& cache, int& data) {
    gui::min_item_size({ BUTTON_BIG, BUTTON_BIG });
    for (int i = 0; i < Cache::SIZE; ++i) {
        if (i) gui::same_line();
        auto const& e = cache.entries[i];
        char str[2];
        sprint_inst_effect_id(str, e.data);
        if (gui::button(str, e.data == data)) {
            data = e.data;
            cache.insert(data);
        }
    }
}



uint8_t  m_track          = 1;
int      m_clavier_offset = 36;
int      m_track_scroll   = 0;

Track    m_copy_track;

Cache    m_inst_cache;
Cache    m_effect_cache;
int      m_instrument = 1;
int      m_effect     = 1;


std::array<bool, TRACK_COUNT> m_empty_tracks;
bool                          m_track_select_allow_nil;
uint8_t*                      m_track_select_value;


void draw_track_select() {
    gui::min_item_size({ edit::screen_size().x, BUTTON_BIG });
    gui::text("SELECT TRACK");

    auto widths = m_track_select_allow_nil ? calculate_column_widths({ -1, -1 }) : calculate_column_widths({ -1 });
    gui::min_item_size({ widths[0], BUTTON_BIG });
    if (gui::button("CANCEL")) {
        edit::set_popup(nullptr);
    }
    if (m_track_select_allow_nil) {
        gui::same_line();
        gui::min_item_size({ widths[1], BUTTON_BIG });
        if (gui::button("CLEAR")) {
            edit::set_popup(nullptr);
            *m_track_select_value = 0;
        }
    }

    gui::separator();

    widths = calculate_column_widths(std::vector<int>(12, -1));
    int track_nr = *m_track_select_value;

    for (int y = 0; y < 21; ++y) {
        for (int x = 0; x < 12; ++x) {
            if (x > 0) gui::same_line();
            int n = x * 21 + y + 1;

            gui::min_item_size({ widths[x], BUTTON_SMALL + 5 });
            char str[3] = "  ";
            sprint_track_id(str, n);

            if (!m_empty_tracks[n - 1]) gui::button_theme(gui::BT_HIGHLIGHT);
            else gui::button_theme(gui::BT_NORMAL);
            if (gui::button(str, n == track_nr)) {
                *m_track_select_value = n;
                edit::set_popup(nullptr);
            }
        }
    }
}


void enter_track_select(uint8_t& dst, bool allow_nil) {
    edit::set_popup(draw_track_select);
    m_track_select_value = &dst;
    m_track_select_allow_nil = allow_nil;

    Song const& song = player::song();
    for (int i = 0; i < (int) m_empty_tracks.size(); ++i) {
        m_empty_tracks[i] = true;
        for (Track::Row const& row : song.tracks[i].rows) {
            if (row.note || row.instrument || row.effect) {
                m_empty_tracks[i] = false;
                break;
            }
        }
    }
}


} // namespace


void enter_track_select(uint8_t& dst) {
    enter_track_select(dst, true);
}

void enter_track_select() {
    enter_track_select(m_track, false);
}

void    select_track(uint8_t t) { m_track = t; }
uint8_t selected_track() { return m_track; }
int     selected_instrument() { return m_instrument; }
void    select_instrument(int i) {
    m_instrument = i;
    m_inst_cache.insert(i);
}
int     selected_effect() { return m_effect; }
void    select_effect(int e) {
    m_effect = e;
    m_effect_cache.insert(e);
}


void sprint_track_id(char* dst, int nr) {
    if (nr == 0) {
        dst[0] = dst[1] == ' ';
    }
    else {
        int x = (nr - 1) / 21;
        int y = (nr - 1) % 21;
        dst[0] = '0' + x + (x > 9) * 7;
        dst[1] = '0' + y + (y > 9) * 7;
    }
    dst[2] = '\0';
}


void sprint_inst_effect_id(char* dst, int nr) {
    dst[0] = " "
        "ABCDEFGHIJKLMNOPQRSTUVWX"
        "YZ@0123456789#$*+=<>!?^~"[nr];
    dst[1] = '\0';
}


void draw_instrument_cache() {
    draw_cache(m_inst_cache, m_instrument);
}


void draw_effect_cache() {
    draw_cache(m_effect_cache, m_effect);
}


void draw_track_view() {
    Song& song = player::song();
    Track& track = song.tracks[m_track - 1];


    // cache
    draw_instrument_cache();
    gui::separator();
    draw_effect_cache();
    gui::separator();

    // track select
    auto widths = calculate_column_widths({ -1, BUTTON_BIG * 5 + gui::SEPARATOR_WIDTH * 2});
    char str[16];
    sprint_track_id(str, m_track);
    gui::min_item_size({ widths[0], BUTTON_BIG });
    if (gui::button(str)) enter_track_select();
    gui::same_line();
    gui::separator();

    gui::min_item_size({ BUTTON_BIG, BUTTON_BIG });
    if (gui::button("<")) track.length = std::max(1, track.length - 1);
    gui::same_line();
    gui::text("%d", track.length);
    gui::same_line();
    if (gui::button(">")) track.length = std::min<int>(MAX_TRACK_LENGTH, track.length + 1);
    gui::same_line();
    gui::separator();

    // copy & paste
    gui::min_item_size({ BUTTON_BIG, BUTTON_BIG });
    if (gui::button(gui::I_COPY)) m_copy_track = track;
    gui::same_line();
    if (gui::button(gui::I_PASTE)) track = m_copy_track;
    gui::min_item_size();
    gui::separator();

    // clavier slider
    gui::drag_theme(gui::DT_SCROLLBAR);
    gui::min_item_size({ edit::screen_size().x, SCROLLBAR_WIDTH });
    gui::drag_int("", "", m_clavier_offset, 0, 96 - gui::CLAVIER_WIDTH, gui::CLAVIER_WIDTH);
    gui::separator();
    Vec c1 = gui::cursor();

    int free_space = edit::screen_size().y - gui::cursor().y - gui::SEPARATOR_WIDTH - BUTTON_BAR;
    int page_length = free_space / BUTTON_SMALL;

    int max_scroll = std::max(0, track.length - page_length);
    m_track_scroll = std::min(m_track_scroll, max_scroll);

    int highlight = settings().track_row_highlight;

    int player_row = player::row();
    for (int i = 0; i < page_length; ++i) {
        int row_nr = m_track_scroll + i;
        if (row_nr >= track.length) {
            gui::padding({ 0, BUTTON_SMALL });
            continue;
        }

        // hightlight
        gui::button_theme(row_nr == player_row    ? gui::BT_CURSOR
                        : row_nr % highlight == 0 ? gui::BT_HIGHLIGHT
                                                  : gui::BT_NORMAL);

        Track::Row& row = track.rows[row_nr];

        // instrument
        sprint_inst_effect_id(str, row.instrument);
        gui::min_item_size({ BUTTON_SMALL, BUTTON_SMALL });
        if (gui::button(str)) {
            if (row.instrument > 0) row.instrument = 0;
            else row.instrument = selected_instrument();
        }
        if (row.instrument > 0 && gui::hold()) {
            select_instrument(row.instrument);
        }

        // effect
        sprint_inst_effect_id(str, row.effect);
        gui::same_line();
        if (gui::button(str)) {
            if (row.effect > 0) row.effect = 0;
            else row.effect = selected_effect();
        }
        if (row.effect > 0 && gui::hold()) {
            select_effect(row.effect);
        }


        // note
        str[0] = str[1] = str[2] = ' ';
        if (row.note == 255) {
            str[0] = str[1] = str[2] = '-';
        }
        else if (row.note > 0) {
            str[0] = "CCDDEFFGGAAB"[(row.note - 1) % 12];
            str[1] = "-#-#--#-#-#-"[(row.note - 1) % 12];
            str[2] = '0' + (row.note - 1) / 12;
        }
        str[3] = '\0';
        gui::same_line();
        gui::min_item_size({ 32, BUTTON_SMALL });
        if (gui::button(str)) {
            if (row.note == 0) row.note = 255;
            else row = {};
        }

        // clavier
        gui::same_line();
        gui::separator();
        int w = edit::screen_size().x - gui::cursor().x - SCROLLBAR_WIDTH - gui::SEPARATOR_WIDTH;
        gui::min_item_size({ w, BUTTON_SMALL });
        uint8_t old_note = row.note;

        if (gui::clavier(row.note, m_clavier_offset)) {
            if (row.note == 0) row = {};
            else if (old_note == 0) {
                row.instrument = selected_instrument();
                row.effect     = selected_effect();
            }
        }

        gui::same_line();
        gui::separator();
        gui::next_line();
    }
    gui::button_theme(gui::BT_NORMAL);

    // track pages
    Vec c2 = gui::cursor();
    gui::cursor({ edit::screen_size().x - SCROLLBAR_WIDTH, c1.y });
    gui::min_item_size({ SCROLLBAR_WIDTH, c2.y - c1.y });
    gui::vertical_drag_int(m_track_scroll, 0, max_scroll, page_length);
    gui::drag_theme(gui::DT_NORMAL);
    gui::cursor(c2);
    gui::separator();
}

