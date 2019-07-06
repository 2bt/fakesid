#include "gui.hpp"
#include "foo.hpp"
#include "debug_renderer.hpp"
#include "renderer.hpp"
#include "app.hpp"


namespace app {
namespace {


bool m_initialized = false;

int m_width;
int m_height;

int m_touch_x;
int m_touch_y;

gfx::Texture2D*   m_canvas;
gfx::Framebuffer* m_framebuffer;


DebugRenderer DBR;
Renderer renderer;

gfx::Texture2D* tex;


} // namespace


bool init() {
    LOGI("app::init");
    if (m_initialized) return true;
    m_initialized = true;

    gfx::init();
    gui::init();

    DBR.init();
    renderer.init();

    m_canvas = gfx::Texture2D::create(gfx::TextureFormat::RGB, 400, 300);
    m_framebuffer = gfx::Framebuffer::create();
    m_framebuffer->attach_color(m_canvas);


    tex = load_texture("gui.png");
    return true;
}


void free() {
    LOGI("app::free");
    if (!m_initialized) return;
    m_initialized = false;

    gui::free();

    DBR.free();
    renderer.free();

    delete m_canvas;
    delete m_framebuffer;
}

void exit() {
    LOGI("app::exit");
    free();
}

void resize(int width, int height) {
    LOGI("app::resize %d %d", width, height);

    m_width  = width;
    m_height = height;
    gfx::screen()->resize(m_width, m_height);
    gui::resize(m_width, m_height);

    DBR.init();
    DBR.origin();
    DBR.translate(-1, -1);
    DBR.scale(2.0 / m_width, 2.0 / m_height);
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

    gfx::screen()->clear({i, i, i, 1});

//    DBR.set_color(255, 255, 255, 255);
//    float d = i * 300;
//    DBR.rect({d, d}, {m_width - d, m_height - d});
//    DBR.set_color(255, 255, 0, 200);
//    glm::vec2 pos = {m_touch_x, m_touch_y};
//    DBR.filled_rect(pos - glm::vec2{100, 100}, pos + glm::vec2{100, 100});
//    DBR.flush();


    m_framebuffer->clear({255, 0, 0, 255});
    DrawContext dc;
//    dc.rect(Rect({10, 10}, {200, 200}), {255, 0, 0, 255});
//    renderer.draw(m_framebuffer, dc.vertices(), tex);
//    dc.clear();

    Vec pos = {50, 10};
    Vec size = {m_canvas->width(), m_canvas->height()};
    dc.rect(Rect(pos, pos + size), {0, 0, 0, 100});
    dc.rect(Rect(pos, pos + size), {255, 255, 255, 255}, {});
    renderer.draw(gfx::screen(), dc.vertices(), m_canvas);

    gui::new_frame();
    gui::button("Ok");
    gui::text("Hello, world!");
    gui::text("foobar\nfoo");
    gui::button("button");
    gui::render();
}


} // namespace
