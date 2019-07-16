#include "edit.hpp"
#include "project_view.hpp"
#include "song_view.hpp"
#include "track_view.hpp"
#include "instrument_effect_view.hpp"
#include "jam_view.hpp"
//#include "help_view.hpp"
#include "player.hpp"
#include "foo.hpp"


#ifdef ANDROID

#include <oboe/Oboe.h>

class : public oboe::AudioStreamCallback {
public:
    oboe::DataCallbackResult onAudioReady(
            oboe::AudioStream *oboeStream,
            void              *audioData,
            int32_t           numFrames) override
    {
        player::fill_buffer((short*) audioData, numFrames);
        return oboe::DataCallbackResult::Continue;
    }
} audio_callback;


bool start_audio() {

    oboe::AudioStreamBuilder builder;
    builder.setDirection(oboe::Direction::Output);
    builder.setSampleRate(MIXRATE);
    builder.setFormat(oboe::AudioFormat::I16);
    builder.setChannelCount(oboe::ChannelCount::Mono);
    builder.setCallback(&audio_callback);
//    builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
//    builder.setSharingMode(oboe::SharingMode::Exclusive);

    oboe::AudioStream* stream = nullptr;
    oboe::Result result = builder.openStream(&stream);
    if (result != oboe::Result::OK) {
        LOGE("openStream: %s", oboe::convertToText(result));
        return false;
    }

//    LOGI("getFramesPerBurst: %d", stream->getFramesPerBurst());
//    stream->setBufferSizeInFrames(stream->getFramesPerBurst() * 4);

    auto rate = stream->getSampleRate();
    if (rate != MIXRATE) {
        LOGW("mixrate is %d but should be %d", rate, MIXRATE);
    }

    result = stream->requestStart();
    if (result != oboe::Result::OK) {
        LOGE("result is not okay: %s", oboe::convertToText(result));
        return false;
    }
    return true;
}

#else

#include <SDL2/SDL.h>
static void audio_callback(void* userdata, Uint8* stream, int len) {
    player::fill_buffer((short*) stream, len / 2);
}

bool start_audio() {
    SDL_AudioSpec spec = { MIXRATE, AUDIO_S16, 1, 0, SAMPLES_PER_FRAME, 0, 0, audio_callback };
    SDL_OpenAudio(&spec, nullptr);
    SDL_PauseAudio(0);
    return true;
}

#endif



namespace edit {
namespace {



Vec   m_screen_size;
EView m_view;
void (*m_popup_func)(void);


} // namespace


void set_view(EView v) {
    m_view = v;
    if (m_view == VIEW_PROJECT) init_project_view();
}


void set_popup(void (*func)(void)) {
    m_popup_func = func;
}

void resize(int width, int height) {
    m_screen_size = { width, height };
}

Vec const& screen_size() {
    return m_screen_size;
}

void init() {
    bool first = true;
    if (first) first = false;
    else return;

    set_view(VIEW_PROJECT);

    init_song(player::song());

    start_audio();

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
        View{ "JAM",    draw_jam_view },
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
    gui::cursor({ 0, m_screen_size.y  - BUTTON_BAR });
    bool block_loop = player::block_loop();
    widths = calculate_column_widths({ -1, -1, -1 });

    // loop
    gui::min_item_size({ widths[0], BUTTON_BAR });
    if (gui::button(gui::I_LOOP, block_loop)) player::block_loop(!block_loop);

    // stop
    gui::same_line();
    gui::min_item_size({ widths[1], BUTTON_BAR });
    if (gui::button(gui::I_STOP)) {
        player::set_playing(false);
        player::reset();
        player::block(get_selected_block());
    }

    // play/pause
    gui::same_line();
    gui::min_item_size({ widths[2], BUTTON_BAR });
    if (gui::button(gui::I_PLAY, player::is_playing())) {
        player::set_playing(!player::is_playing());
    }

}


} // namespace
