#include "gui.hpp"
#include "foo.hpp"
#include "render.hpp"
#include "app.hpp"
#include "edit.hpp"




namespace app {
namespace {


bool              m_initialized = false;
gfx::Framebuffer* m_framebuffer;
gfx::Texture2D*   m_canvas;
float             m_canvas_scale;
int               m_canvas_offset;
Vec               m_canvas_size;


enum {
    WIDTH      = 360,
    MIN_HEIGHT = 590,
};


} // namespace


Vec const& canvas_size() { return m_canvas_size; }


void init() {
    LOGI("app::init");
    if (m_initialized) {
        LOGI("app: already initialized, calling free...");
        free();
        LOGI("app: free done.");
    }
    m_initialized = true;

    gfx::init();
    gui::init();
    render::init();

    resize(gfx::screen()->width(), gfx::screen()->height());

    edit::init();
}


void free() {
    LOGI("app::free");
    if (!m_initialized) return;
    m_initialized = false;

    gui::free();
    render::free();

    delete m_canvas;
    delete m_framebuffer;
    m_canvas      = nullptr;
    m_framebuffer = nullptr;
}


void exit() {
    LOGI("app::exit");
    free();
}


void resize(int width, int height) {
    LOGI("app::resize %d %d", width, height);
    if (!m_initialized) return;

    gfx::screen()->resize(width, height);

    if (m_canvas) {
        delete m_canvas;
        delete m_framebuffer;
    }

    m_canvas_size = {
        WIDTH,
        std::max<int16_t>(WIDTH * height / width, MIN_HEIGHT)
    };
    m_canvas = gfx::Texture2D::create(gfx::TextureFormat::RGBA,
                                      m_canvas_size.x, m_canvas_size.y,
                                      nullptr, gfx::FilterMode::Linear);

    m_framebuffer = gfx::Framebuffer::create();
    m_framebuffer->attach_color(m_canvas);

    // set canvas offset and scale
    glm::vec2 scale = glm::vec2(width, height) / glm::vec2(m_canvas_size);
    if (scale.y < scale.x) {
        m_canvas_offset = (width - WIDTH * scale.y) * 0.5f;
        m_canvas_scale  = scale.y;
    }
    else {
        m_canvas_offset = 0;
        m_canvas_scale  = scale.x;
    }
}


void touch(int x, int y, bool pressed) {
    gui::touch((x - m_canvas_offset) / m_canvas_scale, y / m_canvas_scale, pressed);
}


void draw() {
    gui::begin_frame();

/*
    gui::min_item_size({30, 30});

    static int button_nr = 0;
    for (int i = 0; i < 12; ++i) {
        gui::same_line();
        char t[2];
        sprintf(t, "%X", i);
        if (gui::button(t, button_nr == i)) button_nr = i;
    }
    gui::next_line();
    gui::min_item_size();
    gui::separator();

//    gui::min_item_size({20, 20});
//    gui::button("HALLO");
//    gui::text("%d x %d", gfx::screen()->width(), gfx::screen()->height());
    gui::text(R"(
# 1. INTRODUCTION

Fake SID is a chiptune tracker that let's
you create Commodore 64 music.

At the top of the screen you find certain
tabs, which let you switch to different
views.  Let's go through each view and
discuss them in more detail.


# 2. PROJECT

Here you set the title, author, track
length, and tempo of the current song.
Additionally, songs can be loaded, saved,
deleted, and exported.

*Track length* is the number of rows
per track.  All tracks of a song have the
same length.  As is common with most C64
trackers, time is split into slices of
1/50 of a second, called frames.  *Tempo*
is the number of frames spent per track row.
*Swing* is the number additional frames for
even-numbered track rows.

Pres *New* to reset the current song.
To load a previously saved song, simply
select a song from the song list.  This will
enter the song name in the song name input
field.  Now press *Load*.  Press *Save* to
save the current song under the name in the
input field.  Press *Delete* to delete the
selected song.  You may render the current
song to *WAV* or *OGG* by first selecting
the desired file format and then pressing
*Export*.  Song files and exported songs are
stored in the directories `fakesid/songs`
and `fakesid/exports` of your phone's
internal shared storage.
)");
*/

    edit::draw();

    m_framebuffer->clear({0, 0, 0, 1});
    gui::render(m_framebuffer);


    // render canvas to screen
    gfx::screen()->clear({0, 0, 0, 1});
    render::DrawContext dc;
    dc.copy(Vec(m_canvas_offset, 0), Vec(glm::vec2(m_canvas_size) * m_canvas_scale), {}, m_canvas_size);
    render::draw(gfx::screen(), dc.vertices(), m_canvas);
    dc.clear();
}


} // namespace
