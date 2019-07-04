#include "gui.hpp"
#include "foo.hpp"
#include "debug_renderer.hpp"
#include "app.hpp"

DebugRenderer DBR;

bool App::init() {
    LOGI("App::init");
    if (m_initialized) return true;
    m_initialized = true;

    gfx::init();
    gui::init();

    DBR.init();

    m_canvas = gfx::Texture2D::create(gfx::TextureFormat::RGB, 400, 300);
    m_framebuffer = gfx::Framebuffer::create();
    m_framebuffer->attach_color(m_canvas);

    return true;
}

void App::free() {
    LOGI("App::free");
    if (!m_initialized) return;
    m_initialized = false;

    gui::free();

    DBR.free();

    delete m_canvas;
    delete m_framebuffer;
}

void App::exit() {
    LOGI("App::exit");
    free();
}

void App::resize(int width, int height) {
    LOGI("App::resize %d %d", width, height);

    m_width  = width;
    m_height = height;
    gfx::resize(m_width, m_height);
    gui::resize(m_width, m_height);


    DBR.init();
    DBR.origin();
    DBR.translate(-1, -1);
    DBR.scale(2.0 / m_width, 2.0 / m_height);
}

void App::touch(int x, int y) {
    LOGI("App::touch %d %d", x, y);

    m_touch_x = x;
    m_touch_y = y;
}

void App::draw() {

    static float i = 0;
    i += 0.01;
    if (i > 1) i = 0;

    gfx::clear({i, i, i, 1});

    DBR.set_color(255, 255, 255, 255);
    float d = i * 300;
    DBR.rect({d, d}, {m_width - d, m_height - d});
    DBR.set_color(255, 255, 0, 200);
    glm::vec2 pos = {m_touch_x, m_touch_y};
    DBR.filled_rect(pos - glm::vec2{100, 100}, pos + glm::vec2{100, 100});
    DBR.flush();


    gui::new_frame();
    gui::button("Ok");
    gui::text("Hello, world!");
    gui::text("foobar\nfoo");
    gui::button("button");
    gui::render();
}

