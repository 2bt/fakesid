#include "project_view.hpp"
#include "edit.hpp"
#include "app.hpp"
#include "player.hpp"
//#include "android.hpp"
#include <algorithm>
#include <string>
//#include <dirent.h>
//#include <unistd.h>
//#include <sys/stat.h>
//#include <sndfile.h>


#define FILE_SUFFIX ".sng"


// android
//std::string get_storage_dir();


namespace {

/*
sf_count_t get_filelen(void* user_data) {
    return SDL_RWsize((SDL_RWops*) user_data);
}

sf_count_t seek(sf_count_t offset, int whence, void* user_data) {
    int w = whence == SEEK_CUR ? RW_SEEK_CUR
          : whence == SEEK_SET ? RW_SEEK_SET
                               : RW_SEEK_END;
    return SDL_RWseek((SDL_RWops*) user_data, offset, w);
}

sf_count_t read(void* ptr, sf_count_t count, void* user_data) {
    return SDL_RWread((SDL_RWops*) user_data, ptr, 1, count);
}

sf_count_t write(const void* ptr, sf_count_t count, void* user_data) {
    return SDL_RWwrite((SDL_RWops*) user_data, ptr, 1, count);
}

sf_count_t tell(void* user_data) {
    return SDL_RWtell((SDL_RWops*) user_data);
}
*/


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

//    std::string dst_name = m_songs_dir + name + FILE_SUFFIX;
//    std::string src_name = ASSET_DIR + name + FILE_SUFFIX;

//    struct stat st;
//    if (stat(dst_name.c_str(), &st) != -1) return true;

//    SDL_RWops* src = SDL_RWFromFile(src_name.c_str(), "rb");
//    if (!src) return false;

//    SDL_RWops* dst = SDL_RWFromFile(dst_name.c_str(), "wb");
//    if (!dst) {
//        SDL_RWclose(src);
//        return false;
//    }

//    int len = SDL_RWseek(src, 0, RW_SEEK_END);
//    SDL_RWseek(src, 0, RW_SEEK_SET);
//    std::vector<uint8_t> buffer(len);

//    SDL_RWread(src, buffer.data(), sizeof(uint8_t), buffer.size());
//    SDL_RWclose(src);

//    SDL_RWwrite(dst, buffer.data(), sizeof(uint8_t), buffer.size());
//    SDL_RWclose(dst);

    return true;
}


bool init_dirs() {

//    if (m_root_dir.empty()) {
//        m_root_dir = android::get_storage_dir();
//        if (m_root_dir.empty()) return false;
//    }

//    struct stat st = {};

//    m_songs_dir   = m_root_dir + "/songs/";
//    m_exports_dir = m_root_dir + "/exports/";

//    if (stat(m_root_dir.c_str(), &st) == -1)    mkdir(m_root_dir.c_str(), 0700);
//    if (stat(m_songs_dir.c_str(), &st) == -1)   mkdir(m_songs_dir.c_str(), 0700);
//    if (stat(m_exports_dir.c_str(), &st) == -1) mkdir(m_exports_dir.c_str(), 0700);

//    copy_demo_song("demo1");
//    copy_demo_song("demo2");

    return true;
}


enum ExportFormat { EF_OGG, EF_WAV };

ExportFormat m_export_format = EF_OGG;
//SDL_Thread*  m_export_thread;
bool         m_export_canceled;
bool         m_export_done;
float        m_export_progress;
//SDL_RWops*   m_export_file;
//SNDFILE*     m_export_sndfile;


int export_thread_func(void*) {

//    Song& song = player::song();
//    static std::array<short, 1024> buffer;

//    int frames = (song.track_length * song.tempo + song.track_length / 2 * song.swing) * song.table_length;
//    const int samples = frames * SAMPLES_PER_FRAME;
//    int samples_left = samples;

//    player::block_loop(false);
//    player::set_playing(true);
//    player::reset();

//    while (samples_left > 0 && !m_export_canceled) {
//        int len = std::min<int>(samples_left, buffer.size());
//        samples_left -= len;
//        player::fill_buffer(buffer.data(), len);
//        sf_writef_short(m_export_sndfile, buffer.data(), len);
//        m_export_progress = float(samples - samples_left) / samples;
//    }

//    sf_close(m_export_sndfile);
//    SDL_RWclose(m_export_file);
//    m_export_sndfile = nullptr;
//    m_export_file    = nullptr;

//    player::set_playing(false);
//    player::reset();

//    m_export_done = true;

    return 0;
}


void draw_export_progress() {
    auto widths = calculate_column_widths({ -1 });
    gui::min_item_size({ widths[0], BUTTON_BIG });
    if (gui::button("CANCEL")) m_export_canceled = true;

    gui::min_item_size({ widths[0], BUTTON_BIG });
    gui::text("%3d %%", int(m_export_progress * 100));

    if (m_export_done) {
        int ret;
//        SDL_WaitThread(m_export_thread, &ret);
        edit::set_popup(nullptr);
//        SDL_PauseAudio(0);
        if (m_export_canceled) status("SONG EXPORT WAS CANCELED");
        else status("SONG WAS EXPORTED");
    }
}


bool open_export_file(std::string const& name) {

//    SF_INFO info = { 0, MIXRATE, 1 };
//    std::string path = m_exports_dir + name;
//    if (m_export_format == EF_OGG) {
//        info.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS;
//        path += ".ogg";
//    }
//    else {
//        info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
//        path += ".wav";
//    }

//    m_export_file = SDL_RWFromFile(path.c_str(), "wb");
//    if (!m_export_file) return false;

//    SF_VIRTUAL_IO vio = {
//        get_filelen,
//        seek,
//        read,
//        write,
//        tell,
//    };

//    m_export_sndfile = sf_open_virtual(&vio, SFM_WRITE, &info, m_export_file);
//    if (!m_export_sndfile) {
//        SDL_RWclose(m_export_file);
//        m_export_file = nullptr;
//        return false;
//    }

    // TODO: set quality
    //double quality = 0.8;
    //sf_command(m_export_sndfile, SFC_SET_VBR_ENCODING_QUALITY, &quality, sizeof(quality));

//    return m_export_sndfile != nullptr;
}


void init_export() {

//    std::string name = m_file_name.data();

//    if (name.empty()) {
//        status("Export error: empty song name");
//        return;
//    }

//    if (!open_export_file(name)) {
//        status("Export error: couldn't open file");
//        return;
//    }

    // stop
    //TODO: ensure that the callback has exited
//    SDL_PauseAudio(1);

//    // set meta info
//    Song& song = player::song();
//    sf_set_string(m_export_sndfile, SF_STR_TITLE, song.title.data());
//    sf_set_string(m_export_sndfile, SF_STR_ARTIST, song.author.data());

//    // start thread
//    m_export_canceled = false;
//    m_export_done     = false;
//    m_export_progress = 0;
//    m_export_thread   = SDL_CreateThread(export_thread_func, "song export", nullptr);


//    // popup
//    edit::set_popup(draw_export_progress);
}


enum ConfirmationType {
    CT_NEW,
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


void draw_confirmation() {
    const char* text;
    switch (m_confirmation_type) {
    case CT_DELETE:
        text = "DELETE SONG?";
        break;
    case CT_SAVE:
        text = "OVERRIDE THE EXISTING SONG?";
        break;
    case CT_NEW:
    case CT_LOAD:
    default:
        text = "LOSE CHANGES TO THE CURRENT SONG?";
        break;
    }
    gui::min_item_size({calculate_column_widths({ -1 })[0], BUTTON_BIG });
    gui::text(text);

    auto widths = calculate_column_widths({ -1, -1 });
    gui::min_item_size({ widths[0], BUTTON_BIG });
    if (gui::button("OK")) {
        std::string path = m_songs_dir + m_file_name.data() + FILE_SUFFIX;
        switch (m_confirmation_type) {
        case CT_DELETE:
//            unlink(path.c_str());
            init_project_view();
            status("SONG WAS DELETED");
            break;
        case CT_SAVE:
            save();
            break;
        case CT_NEW:
            init_song(player::song());
            status("SONG WAS RESET");
            break;
        case CT_LOAD:
            if (!load_song(player::song(), path.c_str())) status("Load error: ?");
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
    if (t == CT_LOAD || t == CT_DELETE) {
        if (std::find(m_file_names.begin(), m_file_names.end(), name) == m_file_names.end()) {
            if (t == CT_LOAD) status("LOAD ERROR: SONG NOT LISTED");
            else              status("DELETE ERROR: SONG NOT LISTED");
            return;
        }
    }
    if (t == CT_SAVE) {
        if (name.empty()) {
            status("SAVE ERROR: EMPTY SONG NAME");
            return;
        }
        std::string path = m_songs_dir + name + FILE_SUFFIX;
//        struct stat st;
//        if (stat(path.c_str(), &st) == -1) {
//            // the file doesn't exist yet, so no confirmation needed
//            save();
//            return;
//        }
    }
    m_confirmation_type = t;
    edit::set_popup(draw_confirmation);
}


} // namespace



void init_project_view() {

//    static bool init_dirs_done = false;
//    if (!init_dirs_done) {
//        init_dirs();
//        init_dirs_done = true;
//    }

//    m_file_names.clear();
//    if (DIR* dir = opendir(m_songs_dir.c_str())) {

//        while (struct dirent* ent = readdir(dir)) {
//            if (ent->d_type == DT_REG) {
//                std::string name = ent->d_name;
//                if (name.size() > 4 && name.substr(name.size() - 4) == FILE_SUFFIX) {
//                    m_file_names.emplace_back(name.substr(0, name.size() - 4));
//                }
//            }
//        }
//        closedir(dir);
//        std::sort(m_file_names.begin(), m_file_names.end());
//    }

    m_status_msg = "";
}


void draw_project_view() {

    Song& song = player::song();

    // title and author
    auto widths = calculate_column_widths({ 120, -1 });
    gui::align(gui::LEFT);
    gui::min_item_size({ widths[0], BUTTON_BIG });
    gui::text("TITLE");
    gui::same_line();
    gui::min_item_size({ widths[1], BUTTON_BIG });
    gui::input_text(song.title.data(), song.title.size() - 1);
    gui::min_item_size({ widths[0], BUTTON_BIG });
    gui::text("AUTHOR");
    gui::same_line();
    gui::min_item_size({ widths[1], BUTTON_BIG });
    gui::input_text(song.author.data(), song.author.size() - 1);

    // track length
    auto widths2 = calculate_column_widths({ widths[0], -1, -1, -1 });
    gui::min_item_size({ widths[0], BUTTON_BIG });
    gui::text("TRACK LENGTH");
    gui::same_line();
    gui::align(gui::CENTER);
    gui::min_item_size({ widths2[1], BUTTON_BIG });
    if (gui::button("16", song.track_length == 16)) song.track_length = 16;
    gui::same_line();
    gui::min_item_size({ widths2[2], BUTTON_BIG });
    if (gui::button("24", song.track_length == 24)) song.track_length = 24;
    gui::same_line();
    gui::min_item_size({ widths2[3], BUTTON_BIG });
    if (gui::button("32", song.track_length == 32)) song.track_length = 32;
    gui::align(gui::LEFT);

    // length
    gui::min_item_size({ widths[0], BUTTON_BIG });
    gui::text("SONG LENGTH");
    gui::same_line();
    gui::min_item_size({ widths[1], BUTTON_BIG });
    int frames = (song.track_length * song.tempo + song.track_length / 2 * song.swing) * song.table_length;
    int seconds = frames / FRAMES_PER_SECOND;
    gui::text("%02d:%02d", seconds / 60, seconds % 60);

    // tempo and swing
    widths = calculate_column_widths({ -13, -9 });
    gui::min_item_size({ widths[0], BUTTON_SMALL });
    gui::drag_int("TEMPO", "%d", song.tempo, 4, 16);
    gui::same_line();
    gui::min_item_size({ widths[1], BUTTON_SMALL });
    gui::drag_int("SWING", "%d", song.swing, 0, 8);
    gui::separator();


    // name
    widths = calculate_column_widths({ -1 });
    gui::min_item_size({ widths[0], BUTTON_BIG });
    gui::input_text(m_file_name.data(), m_file_name.size() - 1);
    gui::separator();

    // file select
    enum { PAGE_LENGTH = 10 };
    int max_scroll = std::max<int>(0, m_file_names.size() - PAGE_LENGTH);
    if (m_file_scroll > max_scroll) m_file_scroll = max_scroll;
    gui::same_line();
    Vec c1 = gui::cursor() + Vec(-BUTTON_SMALL, gui::SEPARATOR_WIDTH);
    gui::next_line();
    widths = calculate_column_widths({ -1, gui::SEPARATOR_WIDTH, BUTTON_SMALL });
    for (int i = 0; i < PAGE_LENGTH; ++i) {
        int nr = i + m_file_scroll;
        gui::min_item_size({ widths[0], BUTTON_SMALL });
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
        gui::padding({ widths[2], 0 });
    }
    gui::align(gui::CENTER);

    // scrollbar
    Vec c2 = gui::cursor();
    gui::cursor(c1);
    gui::min_item_size({ BUTTON_SMALL, c2.y - c1.y });
    gui::vertical_drag_int(m_file_scroll, 0, max_scroll, PAGE_LENGTH);
    gui::cursor(c2);
    gui::separator();


    // file buttons
    widths = calculate_column_widths({ -1, -1, -1, -1 });
    gui::min_item_size({ widths[0], BUTTON_BIG });
    if (gui::button("NEW")) init_confirmation(CT_NEW);
    gui::same_line();
    gui::min_item_size({ widths[1], BUTTON_BIG });
    if (gui::button("LOAD")) init_confirmation(CT_LOAD);
    gui::same_line();
    gui::min_item_size({ widths[2], BUTTON_BIG });
    if (gui::button("SAVE")) init_confirmation(CT_SAVE);
    gui::same_line();
    gui::min_item_size({ widths[3], BUTTON_BIG });
    if (gui::button("DELETE")) init_confirmation(CT_DELETE);
    gui::separator();


    widths = calculate_column_widths({ widths2[0], -1, -1, gui::SEPARATOR_WIDTH, -1 });

    gui::min_item_size({ widths2[0], BUTTON_BIG });
    gui::align(gui::LEFT);
    gui::text("FORMAT");
    gui::align(gui::CENTER);
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
    gui::align(gui::LEFT);
    gui::text(m_status_msg.c_str());
    gui::align(gui::CENTER);
    if (++m_status_age > 100) m_status_msg = "";
}

