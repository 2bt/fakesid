#include "project_view.hpp"
#include "edit.hpp"
#include "player.hpp"
#include "settings.hpp"
#include "foo.hpp"
#include <algorithm>
#include <filesystem>
#include <string>
#include <cstring>
#include <sndfile.h>
#include <pthread.h>


#define FILE_SUFFIX ".sng"

namespace fs = std::filesystem;

namespace {

enum SubTab {
    ST_PROJECT,
    ST_SETTINGS
};

SubTab                   m_sub_tab = ST_PROJECT;

int                      m_file_scroll;
std::array<char, 28>     m_file_name;
std::vector<std::string> m_file_names;
std::string              m_root_dir;
std::string              m_songs_dir;
std::string              m_exports_dir;
std::string              m_status_msg;
int                      m_status_age;


void status(std::string const& msg) {
    m_status_msg = msg;
    m_status_age = 0;
}


bool copy_demo_song(std::string const& name) {
    std::string dst_name = m_songs_dir + name + FILE_SUFFIX;
    if (fs::exists(dst_name)) return true;

    std::vector<uint8_t> buffer;
    if (!android::load_asset(name + FILE_SUFFIX, buffer)) {
        return false;
    }

    FILE* file = fopen(dst_name.c_str(), "wb");
    if (!file) return false;
    fwrite(buffer.data(), sizeof(uint8_t), buffer.size(), file);
    fclose(file);

    return true;
}


bool init_dirs() {

    if (m_root_dir.empty()) {
        m_root_dir = android::storage_dir();
    }
    if (m_root_dir.empty()) return false;

    m_songs_dir   = m_root_dir + "/songs/";
    m_exports_dir = m_root_dir + "/exports/";

    fs::create_directories(m_songs_dir);
    fs::create_directories(m_exports_dir);

    copy_demo_song("demo1");
    copy_demo_song("demo2");

    return true;
}


enum ExportFormat { EF_OGG, EF_WAV };

ExportFormat m_export_format = EF_OGG;
bool         m_export_canceled;
bool         m_export_done;
float        m_export_progress;
pthread_t    m_export_thread;
SNDFILE*     m_export_sndfile;


void* export_thread_func(void*) {

    Song& song = player::song();
    static std::array<short, 1024> buffer;

    // count frames of whole song
    int frames = 0;
    for (int i = 0; i < song.table_length; ++i) {
        uint8_t t, s, l = song.block_length(i);
        song.get_block_tempo_and_swing(i, t, s);
        frames += l * t + l / 2 * s;
    }

    const int samples = frames * SAMPLES_PER_FRAME;
    int samples_left = samples;

    player::block(0);
    player::block_loop(false);
    player::set_playing(true);

    while (samples_left > 0 && !m_export_canceled) {
        int len = std::min<int>(samples_left, buffer.size());
        samples_left -= len;
        player::fill_buffer(buffer.data(), len);
        sf_writef_short(m_export_sndfile, buffer.data(), len);
        m_export_progress = float(samples - samples_left) / samples;
    }

    sf_close(m_export_sndfile);
    m_export_sndfile = nullptr;

    player::set_playing(false);
    player::block(0);

    m_export_done = true;
    return nullptr;
}


void draw_export_progress() {
    gui::min_item_size({ edit::screen_size().x, BUTTON_BIG });
    gui::text("EXPORTING SONG");
    gui::min_item_size({ edit::screen_size().x, BUTTON_BIG });
    if (gui::button("CANCEL")) m_export_canceled = true;

    gui::min_item_size({ edit::screen_size().x, BUTTON_BIG });
    gui::text("%3d %%", int(m_export_progress * 100));

    if (m_export_done) {
        pthread_join(m_export_thread, nullptr);
        edit::set_popup(nullptr);
        android::start_audio();

        if (m_export_canceled) status("SONG EXPORT WAS CANCELED");
        else {
            // Export completed successfully, share the file via SAF
            std::string path = m_exports_dir + m_file_name.data();
            path += m_export_format == EF_OGG ? ".ogg" : ".wav";
            android::export_song(path, m_file_name.data());
        }
    }
}


bool open_export_file(std::string const& name) {

    SF_INFO info = { 0, MIXRATE, 1 };
    std::string path = m_exports_dir + name;
    if (m_export_format == EF_OGG) {
        info.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS;
        path += ".ogg";
    }
    else {
        info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
        path += ".wav";
    }

    m_export_sndfile = sf_open(path.c_str(), SFM_WRITE, &info);
    if (!m_export_sndfile) {
        return false;
    }

    // TODO: set quality
    //double quality = 0.8;
    //sf_command(m_export_sndfile, SFC_SET_VBR_ENCODING_QUALITY, &quality, sizeof(quality));

    return true;
}


void init_export() {
    std::string name = m_file_name.data();
    if (name.empty()) {
        status("EXPORT ERROR: EMPTY SONG NAME");
        return;
    }

    if (!open_export_file(name)) {
        status("EXPORT ERROR: COULDN'T OPEN FILE");
        return;
    }


    // stop
    android::stop_audio();

    // set meta info
    Song& song = player::song();
    sf_set_string(m_export_sndfile, SF_STR_TITLE, song.title.data());
    sf_set_string(m_export_sndfile, SF_STR_ARTIST, song.author.data());

    // start thread
    m_export_canceled = false;
    m_export_done     = false;
    m_export_progress = 0;
    int res = pthread_create(&m_export_thread, nullptr, export_thread_func, nullptr);
    if (res != 0) {
        status("EXPORT ERROR: PTHREAD");
        // cleanup
        sf_close(m_export_sndfile);
        m_export_sndfile = nullptr;
        android::start_audio();
        return;
    }

    // popup
    edit::set_popup(draw_export_progress);
}


enum ConfirmationType {
    CT_RESET,
    CT_LOAD,
    CT_SAVE,
    CT_DELETE,
};

ConfirmationType m_confirmation_type;


void save() {
    std::string path = m_songs_dir + m_file_name.data() + FILE_SUFFIX;
    if (!save_song(player::song(), path.c_str())) {
        status("SAVE ERROR: ?");
    }
    else {
        init_project_view();
        status("SONG WAS SAVED");
    }
}


void share_song() {
    std::string name = m_file_name.data();
    if (name.empty()) {
        status("SAVE ERROR: EMPTY FILE NAME");
        return;
    }

    // Save to app storage first
    std::string path = m_songs_dir + name + FILE_SUFFIX;
    if (!save_song(player::song(), path.c_str())) {
        status("SAVE ERROR: ?");
        return;
    }

    // Share via SAF - user picks destination
    android::export_song(path, name);
    init_project_view();
}


void draw_confirmation() {
    const char* text;
    switch (m_confirmation_type) {
    case CT_DELETE:
        text = "DELETE SONG?";
        break;
    case CT_SAVE:
        text = "OVERRIDE THE EXISTING SONG?";
        break;
    case CT_RESET:
    case CT_LOAD:
    default:
        text = "LOSE CHANGES TO THE CURRENT SONG?";
        break;
    }
    gui::min_item_size({edit::screen_size().x, BUTTON_BIG });
    gui::text(text);

    auto widths = calculate_column_widths({ -1, -1 });
    gui::min_item_size({ widths[0], BUTTON_BIG });
    if (gui::button("OK")) {
        std::string path = m_songs_dir + m_file_name.data() + FILE_SUFFIX;
        switch (m_confirmation_type) {
        case CT_DELETE:
            fs::remove(path);
            init_project_view();
            status("SONG WAS DELETED");
            break;
        case CT_SAVE:
            save();
            break;
        case CT_RESET:
            android::stop_audio();
            player::set_playing(false);
            player::block(0);
            android::start_audio();
            init_song(player::song());
            status("SONG WAS RESET");
            break;
        case CT_LOAD:
            android::stop_audio();
            player::set_playing(false);
            player::block(0);
            android::start_audio();
            if (!load_song(player::song(), path.c_str())) status("LOAD ERROR: ?");
            else status("SONG WAS LOADED");
            break;
        }
        edit::set_popup(nullptr);
    }
    gui::same_line();
    gui::min_item_size({ widths[1], BUTTON_BIG });
    if (gui::button("CANCEL")) edit::set_popup(nullptr);
}


void init_confirmation(ConfirmationType t) {
    std::string name = m_file_name.data();
    if (name.empty()) {
        if (t == CT_LOAD)   status("LOAD ERROR: EMPTY SONG NAME");
        if (t == CT_DELETE) status("DELETE ERROR: EMPTY SONG NAME");
        if (t == CT_SAVE)   status("SAVE ERROR: EMPTY SONG NAME");
        return;
    }
    if (t == CT_LOAD || t == CT_DELETE) {
        if (std::find(m_file_names.begin(), m_file_names.end(), name) == m_file_names.end()) {
            if (t == CT_LOAD) status("LOAD ERROR: SONG NOT LISTED");
            else              status("DELETE ERROR: SONG NOT LISTED");
            return;
        }
    }
    if (t == CT_SAVE) {
        std::string path = m_songs_dir + name + FILE_SUFFIX;
        if (!fs::exists(path)) {
            // the file doesn't exist yet, so no confirmation needed
            save();
            return;
        }
    }
    m_confirmation_type = t;
    edit::set_popup(draw_confirmation);
}


void draw_project_tab() {
    Song& song = player::song();

    // title and author
    auto widths = calculate_column_widths({ -1, -3 });
    gui::min_item_size({ widths[0], BUTTON_BIG });
    gui::text("TITLE");
    gui::same_line();
    gui::min_item_size({ widths[1], BUTTON_BIG });
    gui::input_text(song.title);

    gui::separator();
    gui::min_item_size({ widths[0], BUTTON_BIG });
    gui::text("AUTHOR");
    gui::same_line();
    gui::min_item_size({ widths[1], BUTTON_BIG });
    gui::input_text(song.author);

    gui::separator();

    // file name
    gui::min_item_size({ widths[0], BUTTON_BIG });
    gui::text("FILE");
    gui::same_line();
    gui::min_item_size({ widths[1], BUTTON_BIG });
    gui::input_text(m_file_name);
    gui::separator();

    Vec c1 = gui::cursor();

    // file buttons
    gui::min_item_size({ widths[0], BUTTON_BIG });
    if (gui::button("LOAD")) init_confirmation(CT_LOAD);
    if (gui::button("SAVE")) init_confirmation(CT_SAVE);
    if (gui::button("SAVE AS")) share_song();
    if (gui::button("DELETE")) init_confirmation(CT_DELETE);
    if (gui::button("RESET")) init_confirmation(CT_RESET);


    // file select
    gui::cursor({ widths[0], c1.y });
    int free_space = edit::screen_size().y - gui::cursor().y - gui::SEPARATOR_WIDTH * 2 - BUTTON_BAR - BUTTON_BIG * 2;
    int page_length = free_space / BUTTON_SMALL;


    int max_scroll = std::max<int>(0, m_file_names.size() - page_length);
    if (m_file_scroll > max_scroll) m_file_scroll = max_scroll;
    widths = calculate_column_widths({ widths[0], -1, gui::SEPARATOR_WIDTH + SCROLLBAR_WIDTH });

    gui::align(gui::A_LEFT);
    for (int i = 0; i < page_length; ++i) {
        int nr = i + m_file_scroll;
        gui::min_item_size({});
        gui::padding({0, BUTTON_SMALL});
        gui::same_line();
        gui::separator();
        gui::min_item_size({ widths[1] - gui::SEPARATOR_WIDTH, BUTTON_SMALL });
        if (nr < (int) m_file_names.size()) {
            bool select = strcmp(m_file_names[nr].c_str(), m_file_name.data()) == 0;
            if (gui::button(m_file_names[nr].c_str(), select)) {
                strncpy(m_file_name.data(), m_file_names[nr].c_str(), m_file_name.size());
            }
        }
        else {
            gui::padding({});
        }
        gui::same_line();
        gui::separator();
        gui::next_line();
    }
    gui::align(gui::A_CENTER);

    // scrollbar
    Vec c2 = gui::cursor();
    gui::cursor({ edit::screen_size().x - SCROLLBAR_WIDTH, c1.y });
    gui::min_item_size({ SCROLLBAR_WIDTH, c2.y - c1.y });
    gui::drag_theme(gui::DT_SCROLLBAR);
    gui::vertical_drag_int(m_file_scroll, 0, max_scroll, page_length);
    gui::drag_theme(gui::DT_NORMAL);

    gui::cursor({0, c2.y});
    gui::separator();


    widths = calculate_column_widths({ widths[0], -1, -1, gui::SEPARATOR_WIDTH, -1 });

    gui::min_item_size({ widths[0], BUTTON_BIG });
    gui::text("FORMAT");
    gui::same_line();

    gui::min_item_size({ widths[1], BUTTON_BIG });
    if (gui::button("OGG", m_export_format == EF_OGG)) m_export_format = EF_OGG;
    gui::same_line();
    gui::min_item_size({ widths[2], BUTTON_BIG });
    if (gui::button("WAV", m_export_format == EF_WAV)) m_export_format = EF_WAV;
    gui::same_line();
    gui::separator();

    gui::min_item_size({ widths[4], BUTTON_BIG });
    if (gui::button("EXPORT")) init_export();
    gui::separator();

    // status
    widths = calculate_column_widths({ -1 });
    gui::min_item_size({ widths[0], BUTTON_BIG });
    gui::align(gui::A_LEFT);
    gui::text(m_status_msg.c_str());
    gui::align(gui::A_CENTER);
    if (++m_status_age > 100) m_status_msg = "";
}


void draw_settings_tab() {
    gui::min_item_size({ edit::screen_size().x, BUTTON_BIG });
    if (gui::button("PLAY IN BACKGROUND", settings().play_in_background)) settings().play_in_background ^= 1;
    if (gui::button("FULLSCREEN", settings().fullscreen_enabled)) {
        settings().fullscreen_enabled ^= 1;
        android::update_setting(0); // 0 = SETTING_FULLSCREEN_ENABLED
    }
    gui::drag_int("TRACK ROW HIGHLIGHT", "%d", settings().track_row_highlight, 3, 9, 1);
}


} // namespace


void init_project_view() {
    static bool init_dirs_done = false;
    if (!init_dirs_done) {
        init_dirs_done = true;
        init_dirs();
    }

    m_file_names.clear();
    for (auto const& entry : fs::directory_iterator(m_songs_dir)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension().string() != FILE_SUFFIX) continue;
        m_file_names.emplace_back(entry.path().stem().string());
    }
    std::sort(m_file_names.begin(), m_file_names.end());

    m_status_msg = "";
}



void draw_project_view() {

    // tabs
    auto widths = calculate_column_widths({ -1, -1 });
    gui::min_item_size({ widths[0], BUTTON_BIG });
    gui::button_theme(gui::BT_TAB);
    if (gui::button("PROJECT", m_sub_tab == ST_PROJECT)) m_sub_tab = ST_PROJECT;
    gui::same_line();
    gui::min_item_size({ widths[1], BUTTON_BIG });
    if (gui::button("SETTINGS", m_sub_tab == ST_SETTINGS)) m_sub_tab = ST_SETTINGS;
    gui::button_theme(gui::BT_NORMAL);
    gui::separator();

    switch (m_sub_tab) {
    case ST_PROJECT: draw_project_tab(); break;
    case ST_SETTINGS: draw_settings_tab(); break;
    default: break;
    }
}


// called from Java
void import_song(std::string const& path) {
    m_file_name.fill(0);
    if (!load_song(player::song(), path.c_str())) {
        status("IMPORT ERROR");
    }
    else {
        std::string name = fs::path(path).stem().string();
        strncpy(m_file_name.data(), name.c_str(), m_file_name.size() - 1);
        init_project_view();
        status("SONG WAS IMPORTED");
    }
}

