#include "track_view.hpp"
#include "player.hpp"
#include "edit.hpp"
#include <algorithm>


namespace {

bool       m_filter_mode;
Instrument m_copy_inst;
Effect     m_copy_effect;

void draw_instrument_select() {

    auto widths = calculate_column_widths({ -1 });
    gui::min_item_size({ widths[0], BUTTON_BIG });
    gui::text("SELECT INSTRUMENT");

    gui::min_item_size({ widths[0], BUTTON_BIG });
    if (gui::button("CANCEL")) edit::set_popup(nullptr);
    gui::separator();

    Song& song = player::song();

    widths = calculate_column_widths({ -1, -1 });
    for (int y = 0; y < INSTRUMENT_COUNT / 2; ++y) {
        for (int x = 0; x < 2; ++x) {
            int nr = y + x * (INSTRUMENT_COUNT / 2) + 1;
            Instrument const& inst = song.instruments[nr - 1];

            Vec c1 = gui::cursor();
            gui::min_item_size({ widths[x], BUTTON_SMALL });
            if (inst.length > 0 || inst.filter.length > 0) gui::button_theme(gui::BT_HIGHLIGHT);
            else gui::button_theme(gui::BT_NORMAL);
            if (gui::button("", nr == selected_instrument())) {
                edit::set_popup(nullptr);
                select_instrument(nr);
            }

            gui::same_line();
            Vec c2 = gui::cursor();
            gui::cursor(c1);
            gui::min_item_size({ BUTTON_SMALL, BUTTON_SMALL });
            char str[2];
            sprint_inst_effect_id(str, nr);
            gui::text(str);
            gui::same_line();
            gui::min_item_size({ widths[x] - BUTTON_SMALL, BUTTON_SMALL });
            gui::align(gui::A_LEFT);
            gui::text(inst.name.data());
            gui::align(gui::A_CENTER);
            gui::same_line();
            gui::cursor(c2);
        }
        gui::next_line();
    }
    gui::button_theme(gui::BT_NORMAL);
}
// XXX: this is a copy of instrument_select with s/instrument/effect/g :(
void draw_effect_select() {

    auto widths = calculate_column_widths({ -1 });
    gui::min_item_size({ widths[0], BUTTON_BIG });
    gui::text("SELECT EFFECT");

    gui::min_item_size({ widths[0], BUTTON_BIG });
    if (gui::button("CANCEL")) edit::set_popup(nullptr);
    gui::separator();

    Song& song = player::song();

    widths = calculate_column_widths({ -1, -1 });
    for (int y = 0; y < EFFECT_COUNT / 2; ++y) {
        for (int x = 0; x < 2; ++x) {
            int nr = y + x * (EFFECT_COUNT / 2) + 1;
            Effect const& effect = song.effects[nr - 1];

            Vec c1 = gui::cursor();
            gui::min_item_size({ widths[x], BUTTON_SMALL });
            if (effect.length > 0) gui::button_theme(gui::BT_HIGHLIGHT);
            else gui::button_theme(gui::BT_NORMAL);

            if (gui::button("", nr == selected_effect())) {
                edit::set_popup(nullptr);
                select_effect(nr);
            }

            gui::same_line();
            Vec c2 = gui::cursor();
            gui::cursor(c1);
            gui::min_item_size({ BUTTON_SMALL, BUTTON_SMALL });
            char str[2];
            sprint_inst_effect_id(str, nr);
            gui::text(str);
            gui::same_line();
            gui::min_item_size({ widths[x] - BUTTON_SMALL, BUTTON_SMALL });
            gui::align(gui::A_LEFT);
            gui::text(effect.name.data());
            gui::align(gui::A_CENTER);
            gui::same_line();
            gui::cursor(c2);
        }
        gui::next_line();
    }
    gui::button_theme(gui::BT_NORMAL);
}


} // namespace


void enter_instrument_select() {
    edit::set_popup(draw_instrument_select);
}

void enter_effect_select() {
    edit::set_popup(draw_effect_select);
}


enum {
    PAGE_LENGTH = 16
};


void draw_instrument_view() {

    // cache
    draw_instrument_cache();
    gui::separator();

    Song& song = player::song();
    Instrument& inst = song.instruments[selected_instrument() - 1];

    // name
    auto widths = calculate_column_widths({ -1, BUTTON_BIG, BUTTON_BIG });
    gui::min_item_size({ widths[0], BUTTON_BIG });
    gui::align(gui::A_LEFT);
    gui::input_text(inst.name);
    gui::align(gui::A_CENTER);

    // copy & paste
    gui::same_line();
    gui::min_item_size({ BUTTON_BIG, BUTTON_BIG });
    if (gui::button(gui::I_COPY)) m_copy_inst = inst;
    gui::same_line();
    if (gui::button(gui::I_PASTE)) inst = m_copy_inst;
    gui::separator();


    // wave/filter switch
    widths = calculate_column_widths({ -1, -1 });
    gui::min_item_size({ widths[0], BUTTON_BIG });
    if (inst.length > 0) gui::button_theme(gui::BT_TAB_HIGHLIGHT);
    else gui::button_theme(gui::BT_TAB);
    if (gui::button("WAVE", !m_filter_mode)) m_filter_mode = false;
    gui::same_line();
    gui::min_item_size({ widths[1], BUTTON_BIG });
    if (inst.filter.length > 0) gui::button_theme(gui::BT_TAB_HIGHLIGHT);
    else gui::button_theme(gui::BT_TAB);
    if (gui::button("FILTER", m_filter_mode)) m_filter_mode = true;
    gui::button_theme(gui::BT_NORMAL);
    gui::separator();


    if (!m_filter_mode) {

        // adsr
        constexpr char const* labels[] = { "ATTACK", "DECAY", "SUSTAIN", "RELEASE" };
        widths = calculate_column_widths({ -1, -1, BUTTON_BIG });
        for (int i = 0; i < 2; ++i) {
            if (i > 0) gui::same_line();
            gui::min_item_size({ widths[i], BUTTON_SMALL });
            gui::drag_int(labels[i], "%X", inst.adsr[i], 0, 15);
        };
        Vec c = gui::cursor();
        gui::same_line();
        gui::min_item_size({ BUTTON_BIG, BUTTON_SMALL * 2 });
        if (gui::button(gui::I_STOP, inst.hard_restart)) inst.hard_restart ^= 1;
        gui::cursor(c);
        for (int i = 0; i < 2; ++i) {
            if (i > 0) gui::same_line();
            gui::min_item_size({ widths[i], BUTTON_SMALL });
            gui::drag_int(labels[i + 2], "%X", inst.adsr[i + 2], 0, 15);
        };

        gui::min_item_size({ edit::screen_size().x, 0 });
        gui::separator();

        // wave
        for (int i = 0; i < MAX_INSTRUMENT_LENGTH; ++i) {

            char str[2];
            snprintf(str, 2, "%X", i);
            gui::min_item_size({ BUTTON_SMALL, BUTTON_SMALL });

            if (i >= inst.length) gui::text(str);
            else if (gui::button(str, i == inst.loop)) inst.loop = i;

            gui::same_line();
            gui::separator();
            if (i >= inst.length) {
                gui::next_line();
                continue;
            }
            auto& row = inst.rows[i];

            // flags
            constexpr std::pair<uint8_t, gui::Icon> flags[] = {
                { Instrument::F_NOISE, gui::I_NOISE },
                { Instrument::F_PULSE, gui::I_PULSE },
                { Instrument::F_SAW,   gui::I_SAW },
                { Instrument::F_TRI,   gui::I_TRI },
                { Instrument::F_RING,  gui::I_RING },
                { Instrument::F_SYNC,  gui::I_SYNC },
                { Instrument::F_GATE,  gui::I_GATE },
            };
            gui::min_item_size({ BUTTON_SMALL, BUTTON_SMALL });
            for (auto p : flags) {
                gui::same_line();
                if (gui::button(p.second, row.flags & p.first)) row.flags ^= p.first;
            }

            gui::same_line();
            gui::separator();
            str[0] = "+=-"[row.operation];
            str[1] = '\0';
            gui::min_item_size({ BUTTON_SMALL, BUTTON_SMALL });
            if (gui::button(str)) row.operation = !row.operation;
            gui::same_line();
            gui::min_item_size({ edit::screen_size().x - gui::cursor().x, BUTTON_SMALL });
            gui::drag_int("", "%02X", row.value, 0, 31);
        }

        gui::min_item_size({ edit::screen_size().x, 0 });
        gui::separator();

        gui::min_item_size({ BUTTON_BIG, BUTTON_BIG });
        if (gui::button(gui::I_DELETE_ROW)) {
            if (inst.loop < inst.length) {
                inst.rows[inst.loop] = {};
                std::rotate(
                    inst.rows.begin() + inst.loop,
                    inst.rows.begin() + inst.loop + 1,
                    inst.rows.begin() + inst.length);
                --inst.length;
                if (inst.loop > 0 && inst.loop == inst.length) --inst.loop;
            }
        }
        gui::same_line();
        if (gui::button(gui::I_ADD_ROW_ABOVE)) {
            if (inst.length == 0) {
                inst.rows[0] = {};
                inst.length = 1;
                inst.loop = 0;
            }
            else if (inst.loop <= inst.length && inst.length < inst.rows.size()) {
                inst.rows[inst.length] = {};
                std::rotate(
                    inst.rows.begin() + inst.loop,
                    inst.rows.begin() + inst.length,
                    inst.rows.begin() + inst.length + 1);
                ++inst.length;
                ++inst.loop;
            }
        }
        gui::same_line();
        if (gui::button(gui::I_ADD_ROW_BELOW)) {
            if (inst.length == 0) {
                inst.rows[0] = {};
                inst.length = 1;
                inst.loop = 0;
            }
            else if (inst.loop < inst.length && inst.length < inst.rows.size()) {
                inst.rows[inst.length] = {};
                std::rotate(
                    inst.rows.begin() + inst.loop + 1,
                    inst.rows.begin() + inst.length,
                    inst.rows.begin() + inst.length + 1);
                ++inst.length;
            }
        }

    }
    else {
        // filter table
        Filter& filter = inst.filter;

        widths = calculate_column_widths({ -1, -1, -1, -1 });
        char str[] = "VOICE .";
        for (int c = 0; c < CHANNEL_COUNT; ++c) {
            str[6] = '0' + c;
            if (c) gui::same_line();
            gui::min_item_size({ widths[c], BUTTON_SMALL });
            if (gui::button(str, filter.routing & (1 << c))) filter.routing ^= 1 << c;
        }
        gui::separator();

        for (int i = 0; i < MAX_FILTER_LENGTH; ++i) {

            snprintf(str, 2, "%X", i);
            gui::min_item_size({ BUTTON_SMALL, BUTTON_SMALL });

            if (i >= filter.length) gui::text(str);
            else if (gui::button(str, i == filter.loop)) filter.loop = i;

            gui::same_line();
            gui::separator();
            if (i >= filter.length) {
                gui::next_line();
                continue;
            }
            auto& row = filter.rows[i];


            // type
            gui::same_line();
            gui::min_item_size({ BUTTON_SMALL, BUTTON_SMALL });
            if (gui::button(gui::I_LOWPASS, row.type & FILTER_LOW)) row.type ^= FILTER_LOW;
            gui::same_line();
            gui::min_item_size({ BUTTON_SMALL, BUTTON_SMALL });
            if (gui::button(gui::I_BANDPASS, row.type & FILTER_BAND)) row.type ^= FILTER_BAND;
            gui::same_line();
            gui::min_item_size({ BUTTON_SMALL, BUTTON_SMALL });
            if (gui::button(gui::I_HIGHPASS, row.type & FILTER_HIGH)) row.type ^= FILTER_HIGH;

            // resonance
            gui::same_line();
            gui::separator();

            gui::min_item_size({ 90, BUTTON_SMALL });
            gui::drag_int("", "%X", row.resonance, 0, 15);


            // operation
            gui::same_line();
            gui::separator();
            str[0] = "+=-"[row.operation];
            str[1] = '\0';
            gui::min_item_size({ BUTTON_SMALL, BUTTON_SMALL });
            if (gui::button(str)) row.operation = (row.operation + 1) % 3;

            // cutoff
            gui::same_line();
            gui::min_item_size({ edit::screen_size().x - gui::cursor().x, BUTTON_SMALL });
            gui::drag_int("", "%02X", row.value, 0, 31);
        }

        gui::min_item_size({ edit::screen_size().x, 0 });
        gui::separator();

        gui::min_item_size({ BUTTON_BIG, BUTTON_BIG });
        if (gui::button(gui::I_DELETE_ROW)) {
            if (filter.loop < filter.length) {
                std::rotate(
                    filter.rows.begin() + filter.loop,
                    filter.rows.begin() + filter.loop + 1,
                    filter.rows.begin() + filter.length);
                --filter.length;
                if (filter.loop > 0 && filter.loop == filter.length) --filter.loop;
            }
        }
        gui::same_line();
        if (gui::button(gui::I_ADD_ROW_ABOVE)) {
            if (filter.length == 0) {
                filter.rows[0] = {};
                filter.length = 1;
                filter.loop = 0;
            }
            else if (filter.loop <= filter.length && filter.length < filter.rows.size()) {
                filter.rows[filter.length] = {};
                std::rotate(
                    filter.rows.begin() + filter.loop,
                    filter.rows.begin() + filter.length,
                    filter.rows.begin() + filter.length + 1);
                ++filter.length;
                ++filter.loop;
            }
        }
        gui::same_line();
        if (gui::button(gui::I_ADD_ROW_BELOW)) {
            if (filter.length == 0) {
                filter.rows[0] = {};
                filter.length = 1;
                filter.loop = 0;
            }
            else if (filter.loop < filter.length && filter.length < filter.rows.size()) {
                filter.rows[filter.length] = {};
                std::rotate(
                    filter.rows.begin() + filter.loop + 1,
                    filter.rows.begin() + filter.length,
                    filter.rows.begin() + filter.length + 1);
                ++filter.length;
            }
        }

    }
    gui::min_item_size({ edit::screen_size().x, 0 });
    gui::separator();
}


void draw_effect_view() {

    // cache
    draw_effect_cache();
    gui::separator();


    Song& song = player::song();
    Effect& effect = song.effects[selected_effect() - 1];

    // name
    auto widths = calculate_column_widths({ -1, BUTTON_BIG, BUTTON_BIG });
    gui::min_item_size({ widths[0], BUTTON_BIG });
    gui::align(gui::A_LEFT);
    gui::input_text(effect.name);
    gui::align(gui::A_CENTER);

    // copy & paste
    gui::same_line();
    gui::min_item_size({ BUTTON_BIG, BUTTON_BIG });
    if (gui::button(gui::I_COPY)) m_copy_effect = effect;
    gui::same_line();
    gui::min_item_size({ BUTTON_BIG, BUTTON_BIG });
    if (gui::button(gui::I_PASTE)) effect = m_copy_effect;

    gui::separator();

    widths = calculate_column_widths({ BUTTON_SMALL, gui::SEPARATOR_WIDTH, BUTTON_SMALL, -1 });

    // rows
    for (int i = 0; i < MAX_EFFECT_LENGTH; ++i) {
        char str[2];
        snprintf(str, 2, "%X", i);
        gui::min_item_size({ BUTTON_SMALL, BUTTON_SMALL });

        if (i >= effect.length) gui::text(str);
        else if (gui::button(str, i == effect.loop)) effect.loop = i;

        gui::same_line();
        gui::separator();
        if (i >= effect.length) {
            gui::padding({});
            continue;
        }

        auto& row = effect.rows[i];

        str[0] = "+=~"[row.operation];
        gui::min_item_size({ BUTTON_SMALL, BUTTON_SMALL });
        if (gui::button(str)) {
            row.operation = (row.operation + 1) % 3;
            row.value = 0x30;
        }

        int min_value;
        int max_value;

        gui::same_line();
        gui::min_item_size({ widths[3], BUTTON_SMALL });
        if (row.operation == Effect::OP_RELATIVE || row.operation == Effect::OP_DETUNE) {
            int v = row.value - 0x30;
            gui::id(&row.value);
            if (gui::drag_int("", "%d", v, -24, 24)) row.value = v + 0x30;
            min_value = 0x30 - 24;
            max_value = 0x30 + 24;
        }
        else {
            min_value = 0;
            max_value = 96;
            gui::drag_int("", "%d", row.value, min_value, max_value, 2);
        }
    }

    gui::min_item_size({ edit::screen_size().x, 0 });
    gui::separator();


    gui::min_item_size({ BUTTON_BIG, BUTTON_BIG });
    if (gui::button(gui::I_DELETE_ROW)) {
        if (effect.loop < effect.length) {
            effect.rows[effect.loop] = {};
            std::rotate(
                effect.rows.begin() + effect.loop,
                effect.rows.begin() + effect.loop + 1,
                effect.rows.begin() + effect.length);
            --effect.length;
            if (effect.loop > 0 && effect.loop == effect.length) --effect.loop;
        }
    }
    gui::same_line();
    if (gui::button(gui::I_ADD_ROW_ABOVE)) {
        if (effect.length == 0) {
            effect.rows[0] = {};
            effect.length = 1;
            effect.loop = 0;
        }
        else if (effect.loop <= effect.length && effect.length < effect.rows.size()) {
            effect.rows[effect.length] = {};
            std::rotate(
                effect.rows.begin() + effect.loop,
                effect.rows.begin() + effect.length,
                effect.rows.begin() + effect.length + 1);
            ++effect.length;
            ++effect.loop;
        }
    }
    gui::same_line();
    if (gui::button(gui::I_ADD_ROW_BELOW)) {
        if (effect.length == 0) {
            effect.rows[0] = {};
            effect.length = 1;
            effect.loop = 0;
        }
        else if (effect.loop < effect.length && effect.length < effect.rows.size()) {
            effect.rows[effect.length] = {};
            std::rotate(
                effect.rows.begin() + effect.loop + 1,
                effect.rows.begin() + effect.length,
                effect.rows.begin() + effect.length + 1);
            ++effect.length;
        }
    }


    gui::min_item_size({ edit::screen_size().x, 0 });
    gui::separator();
}

