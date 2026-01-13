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
int               m_inset_top    = 0;
int               m_inset_bottom = 0;
bool              m_canvas_setup_requested = false;


void setup_canvas() {
    int width  = gfx::screen()->width();
    int height = gfx::screen()->height();

    // Account for system insets (status bar, navigation bar)
    int available_height = height - m_inset_top - m_inset_bottom;

    Vec s = {
        WIDTH,
        std::max<int16_t>(WIDTH * available_height / width, MIN_HEIGHT),
    };
    edit::resize(s.x, s.y);

    if (m_canvas) {
        delete m_canvas;
        delete m_framebuffer;
    }

    Vec t = { 2, 2 };
    while (t.x < s.x) t.x *= 2;
    while (t.y < s.y) t.y *= 2;
    m_canvas = gfx::Texture2D::create(gfx::TextureFormat::RGBA, t.x, t.y, nullptr,
#ifdef ANDROID
                                      gfx::FilterMode::Nearest
#else
                                      gfx::FilterMode::Linear
#endif
                                      );
    m_framebuffer = gfx::Framebuffer::create();
    m_framebuffer->attach_color(m_canvas);

    // set canvas offset and scale
    glm::vec2 scale = glm::vec2(width, available_height) / glm::vec2(s);
    if (scale.y < scale.x) {
        m_canvas_offset = (width - s.x * scale.y) * 0.5f;
        m_canvas_scale  = scale.y;
    }
    else {
        m_canvas_offset = 0;
        m_canvas_scale  = scale.x;
    }
}

} // namespace



void init() {
    free();

    gfx::init();
    gui::init();
    render::init();

    resize(gfx::screen()->width(), gfx::screen()->height());

    edit::init();

    m_initialized = true;
}


void free() {
    if (!m_initialized) return;
    m_initialized = false;

    gui::free();
    render::free();

    delete m_canvas;
    delete m_framebuffer;
    m_canvas      = nullptr;
    m_framebuffer = nullptr;
}


void resize(int width, int height) {
    if (!m_initialized) return;
    width  = std::max(1, width);
    height = std::max(1, height);
    gfx::screen()->resize(width, height);
    m_canvas_setup_requested = true;
}

void set_insets(int top_inset, int bottom_inset) {
    LOGD("app::set_insets %d %d", top_inset, bottom_inset);
    m_inset_top    = top_inset;
    m_inset_bottom = bottom_inset;
    m_canvas_setup_requested = true;
}


void touch(int x, int y, bool pressed) {
    gui::touch((x - m_canvas_offset) / m_canvas_scale, (y - m_inset_top) / m_canvas_scale, pressed);
}


void key(int key, int unicode) {
    gui::key(key, unicode);
}


void draw() {
    // setup canvas if size or insets changed
    if (m_canvas_setup_requested) {
        m_canvas_setup_requested = false;
        setup_canvas();
    }

    gui::begin_frame();

    edit::draw();

    m_framebuffer->clear({0, 0, 0, 1});
    gui::render(m_framebuffer);

    // render canvas to screen
    gfx::screen()->clear({0, 0, 0, 1});
    render::DrawContext dc;
    Vec s = { m_canvas->width(),  m_canvas->height() };
    dc.copy(Vec(m_canvas_offset, m_inset_top), Vec(glm::vec2(s) * m_canvas_scale), {}, s);
    render::draw(gfx::screen(), dc, m_canvas);
    dc.clear();
}


} // namespace
