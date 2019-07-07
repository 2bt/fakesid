#include "gui.hpp"
#include "foo.hpp"
#include "render.hpp"
#include "app.hpp"

enum {
    WIDTH      = 360,
    MIN_HEIGHT = 590,
};

namespace app {
namespace {


bool              m_initialized = false;
gfx::Framebuffer* m_framebuffer;
gfx::Texture2D*   m_canvas;
float             m_canvas_scale;
int               m_canvas_offset;

} // namespace


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

    int h = std::max<int>(WIDTH * height / width, MIN_HEIGHT);
    m_canvas = gfx::Texture2D::create(gfx::TextureFormat::RGBA, WIDTH, h);
    m_framebuffer = gfx::Framebuffer::create();
    m_framebuffer->attach_color(m_canvas);

    // set canvas offset and scale
    glm::vec2 scale = glm::vec2(width, height) / glm::vec2(WIDTH, h);
    if (scale.y < scale.x) {
        m_canvas_offset = (width - WIDTH * scale.y) * 0.5f;
        m_canvas_scale  = scale.y;
    }
    else {
        m_canvas_offset = 0;
        m_canvas_scale  = scale.y;
    }

}

void touch(int x, int y) {
    LOGI("app::touch %d %d", x, y);


    gui::touch((x - m_canvas_offset) / m_canvas_scale, y / m_canvas_scale);
}

void draw() {

    // render to canvas
    Vec canvas_size = {m_canvas->width(), m_canvas->height()};
    gui::begin_frame();


    gui::min_item_size({30, 30});
    for (int i = 0; i < 12; ++i) {
        gui::same_line();
        char t[3];
        sprintf(t, "%02X", i);
        gui::button(t);
    }
    gui::next_line();
    gui::min_item_size();
    gui::separator();

    gui::min_item_size({20, 20});
    gui::button("HALLO");
    gui::text("%d x %d", m_canvas->width(), m_canvas->height());
    gui::text("%d x %d", gfx::screen()->width(), gfx::screen()->height());




    m_framebuffer->clear({0, 0, 0, 1});
    gui::render(m_framebuffer);


    // render canvas to screen
    gfx::screen()->clear({0, 0, 0, 1});
    render::DrawContext dc;
    dc.copy(Vec(m_canvas_offset, 0), Vec(glm::vec2(canvas_size) * m_canvas_scale), {}, canvas_size);
    render::draw(gfx::screen(), dc.vertices(), m_canvas);
    dc.clear();
}


} // namespace
