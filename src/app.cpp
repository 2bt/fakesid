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
    Vec s = { 2, 2 };
    while (s.x < m_canvas_size.x) s.x *= 2;
    while (s.y < m_canvas_size.y) s.y *= 2;
    m_canvas = gfx::Texture2D::create(gfx::TextureFormat::RGBA, s.x, s.y);

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


void key(int key, int unicode) {
    LOGI("app::key %d %d", key, unicode);
    gui::key(key, unicode);
}


void draw() {
    gui::begin_frame();

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
