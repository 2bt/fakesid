#include "gui.hpp"
#include "foo.hpp"
#include "debug_renderer.hpp"
#include "render.hpp"
#include "app.hpp"


enum {
    WIDTH      = 360,
    MIN_HEIGHT = 620,
};

namespace app {
namespace {


bool m_initialized = false;

int m_touch_x;
int m_touch_y;

gfx::Texture2D*   m_canvas;
gfx::Framebuffer* m_framebuffer;


DebugRenderer DBR;

gfx::Texture2D* tex;


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

    DBR.init();

    tex = load_texture("gui.png");

    resize(gfx::screen()->width(), gfx::screen()->height());
}


void free() {
    LOGI("app::free");
    if (!m_initialized) return;
    m_initialized = false;

    gui::free();
    render::free();

    DBR.free();

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
    gui::resize(width, height);

    if (m_canvas) {
        delete m_canvas;
        delete m_framebuffer;
    }

    int h = std::max<int>(WIDTH * height / width, MIN_HEIGHT);
    m_canvas = gfx::Texture2D::create(gfx::TextureFormat::RGBA, WIDTH, h);
    m_framebuffer = gfx::Framebuffer::create();
    m_framebuffer->attach_color(m_canvas);

    DBR.origin();
    DBR.translate(-1, -1);
    DBR.scale(2.0 / width, 2.0 / height);
}

void touch(int x, int y) {
    LOGI("app::touch %d %d", x, y);

    m_touch_x = x;
    m_touch_y = y;
}

void draw() {

    static float i = 0;
    i += 0.01;
    if (i > 1) i = 0;


    Vec screen_size = {gfx::screen()->width(), gfx::screen()->height()};
    Vec canvas_size = {m_canvas->width(), m_canvas->height()};

    // render to canvas
    m_framebuffer->clear({0.3, 0.3, 0.3, 1});
    render::DrawContext dc;
    dc.rect({10, 10}, canvas_size - Vec(20), {200, 0, 0, 255});
    dc.rect({10, 10}, {50, 50}, {0, 0});



    dc.rect({200 + i * 100, 10}, {50, 50}, {255, 200, 0, 255});

    render::draw(m_framebuffer, dc.vertices(), tex);
    dc.clear();



    // render canvas to screen
    gfx::screen()->clear({0, 0, 0, 1});
    glm::vec2 scale = glm::vec2(screen_size) / glm::vec2(canvas_size);
    Vec pos, size;
    if (scale.y < scale.x) {
        pos = Vec((screen_size.x - canvas_size.x * scale.y) * 0.5f, 0);
        size = Vec(glm::vec2(canvas_size) * scale.y);
    }
    else {
        size = Vec(glm::vec2(canvas_size) * scale.x);
    }


    dc.rect(pos, size, {}, canvas_size);
    render::draw(gfx::screen(), dc.vertices(), m_canvas);
    dc.clear();

//    gui::new_frame();
//    gui::button("Ok");
//    gui::text("Hello, world!");
//    gui::text("foobar\nfoo");
//    gui::button("button");
//    gui::render();
}


} // namespace
