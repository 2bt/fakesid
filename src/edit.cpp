#include "edit.hpp"
#include "project_view.hpp"
#include "song_view.hpp"
#include "track_view.hpp"
#include "instrument_effect_view.hpp"
//#include "jam_view.hpp"
//#include "help_view.hpp"
#include "app.hpp"
#include "player.hpp"



namespace edit {
namespace {

//void audio_callback(void* userdata, Uint8* stream, int len) {
//    player::fill_buffer((short*) stream, len / 2);
//}

EView m_view;
void (*m_popup_func)(void);


} // namespace


void set_view(EView v) {
    m_view = v;
//    if (m_view == VIEW_PROJECT) init_project_view();
}


void set_popup(void (*func)(void)) {
    m_popup_func = func;
}


void init() {
    bool first = true;
    if (first) first = false;
    else return;

    set_view(VIEW_PROJECT);

    init_song(player::song());

//    SDL_AudioSpec spec = { MIXRATE, AUDIO_S16, 1, 0, SAMPLES_PER_FRAME, 0, 0, audio_callback };
//    SDL_OpenAudio(&spec, nullptr);
//    SDL_PauseAudio(0);
    return;
}

void free() {
//    SDL_CloseAudio();
}

void draw() {

    if (m_popup_func) {
        m_popup_func();
        return;
    }

    // view select buttons
    struct View {
        char const* name;
        void (*draw)(void);
    };
    constexpr std::array<View, 7> views = {
        View{ "PROJ",   draw_project_view },
        View{ "SONG",   draw_song_view },
        View{ "TRACK",  draw_track_view },
        View{ "INSTR",  draw_instrument_view },
        View{ "EFFECT", draw_effect_view },
        View{ "JAM",    0 },
        View{ "?",      0 },
    };
    std::vector<int> weights = std::vector<int>(views.size() - 1, -1);
    weights.push_back(BUTTON_BIG);

    auto widths = calculate_column_widths(weights);


    // top bar
    gui::button_style(gui::BS_TAB);
    for (int i = 0; i < (int) views.size(); ++i) {
        if (i) gui::same_line();
        gui::min_item_size({ widths[i], BUTTON_BAR });
        bool button = gui::button(views[i].name, m_view == i);
        bool hold   = m_view != i && (i == VIEW_INSTRUMENT || i == VIEW_EFFECT) && gui::hold();
        if (button && m_view != i) {
            // switch view
            set_view((EView) i);
        }
        else if ((button && m_view == i) || hold) {
            // open select menu
            switch (i) {
            case VIEW_TRACK:
                enter_track_select();
                break;
            case VIEW_INSTRUMENT:
                enter_instrument_select();
                break;
            case VIEW_EFFECT:
                enter_effect_select();
                break;
            default: break;
            }
        }
    }
    gui::button_style(gui::BS_NORMAL);
    gui::min_item_size();
    gui::separator();

    // XXX
    if (views[m_view].draw)
    views[m_view].draw();



    // bottom bar
    gui::cursor({ 0, app::canvas_size().y  - BUTTON_BAR });
    bool block_loop = player::block_loop();
    widths = calculate_column_widths({ -1, -1, -1 });

    // loop
    gui::min_item_size({ widths[0], BUTTON_BAR });
    if (gui::button("\x13", block_loop)) player::block_loop(!block_loop);

    // stop
    gui::same_line();
    gui::min_item_size({ widths[1], BUTTON_BAR });
    if (gui::button("\x11")) {
        player::set_playing(false);
        player::reset();
//        player::block(get_selected_block());
    }

    // play/pause
    gui::same_line();
    gui::min_item_size({ widths[2], BUTTON_BAR });
    if (gui::button("\x10\x12", player::is_playing())) {
        player::set_playing(!player::is_playing());
    }

}


} // namespace
