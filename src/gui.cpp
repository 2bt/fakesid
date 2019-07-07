#include "gui.hpp"
#include "gfx.hpp"
#include "foo.hpp"
#include <cstdarg>
#include <algorithm>
#include <array>
#include <memory>


namespace gui {
namespace {


using Col = glm::u8vec4;


enum {
    FONT_WIDTH  = 7,
    FONT_HEIGHT = 12,
};


struct Rect {
    Rect() {}
    Rect(const Vec& min, const Vec& max) : min(min), max(max) {}
    Vec tl() const { return min; }
    Vec tr() const { return Vec(max.x, min.y); }
    Vec bl() const { return Vec(min.x, max.y); }
    Vec br() const { return max; }
    Vec center() const { Vec v = min + max; return Vec(v.x / 2, v.y / 2); }
    Vec size() const { return max - min; }
    Rect expand(short d) const { return { min - Vec(d), max + Vec(d) }; }
    Rect expand(const Vec& d) const { return { min - d, max + d }; }
    bool contains(const Vec& p) {
        return p.x >= min.x && p.y >= min.y &&
               p.x <  max.x && p.y <  max.y;
    }

    Vec min;
    Vec max;
};


enum RectStyle {
    RECT_FILL,
    RECT_FILL_ROUND_1,
    RECT_FILL_ROUND_2,
    RECT_FILL_ROUND_3,
    RECT_STROKE,
    RECT_STROKE_ROUND_1,
    RECT_STROKE_ROUND_2,
    RECT_STROKE_ROUND_3,
};


struct Vertex {
    Vec pos;
    Vec uv;
    Col col;
};


class DrawContext {
public:

    void set_active(bool a) { m_active = a; }

    void clear() { m_vertices.clear(); }

    std::vector<Vertex> const& vertices() const { return m_vertices; }

    void draw_rect(const Rect& rect, const Col& color) {
        if (!m_active) return;
        Vertex vs[] = {
            { rect.tl(), {0, 0}, color },
            { rect.bl(), {0, 1}, color },
            { rect.tr(), {1, 0}, color },
            { rect.br(), {1, 1}, color },
        };
        draw_quad(vs[0], vs[1], vs[2], vs[3]);
    }

    void draw_rect(const Rect& rect, const Col& color, const Vec& uv) {
        if (!m_active) return;
        Vec s = rect.size();
        Vertex vs[] = {
            { rect.tl(), uv, color },
            { rect.bl(), uv + Vec(0, s.y), color },
            { rect.tr(), uv + Vec(s.x, 0), color },
            { rect.br(), uv + s, color },
        };
        draw_quad(vs[0], vs[1], vs[2], vs[3]);
    }

    void draw_rect(const Rect& rect, const Col& color, RectStyle style) {
        if (!m_active) return;
        if (style == 0) {
            draw_rect(rect, color);
            return;
        }
        Vec o = { 16 * style, 0 };
        Vec u = { 7, 0 };
        Vec v = { 0, 7 };
        Vertex vs[] = {
            { rect.tl(),     o,     color },
            { rect.tl() + v, o + v, color },
            { rect.bl() - v, o + v, color },
            { rect.bl(),     o,     color },
            { rect.tl() + u,     o + u,     color },
            { rect.tl() + u + v, o + u + v, color },
            { rect.bl() + u - v, o + u + v, color },
            { rect.bl() + u,     o + u,     color },
            { rect.tr() - u,     o + u,     color },
            { rect.tr() - u + v, o + u + v, color },
            { rect.br() - u - v, o + u + v, color },
            { rect.br() - u,     o + u,     color },
            { rect.tr(),     o,     color },
            { rect.tr() + v, o + v, color },
            { rect.br() - v, o + v, color },
            { rect.br(),     o,     color },
        };
        draw_quad(vs[0], vs[1], vs[4], vs[5]);
        draw_quad(vs[1], vs[2], vs[5], vs[6]);
        draw_quad(vs[2], vs[3], vs[6], vs[7]);
        draw_quad(vs[4], vs[5], vs[8], vs[9]);
        if (style < RECT_STROKE) draw_quad(vs[5], vs[6], vs[9], vs[10]);
        draw_quad(vs[6], vs[7], vs[10], vs[11]);
        draw_quad(vs[8], vs[9], vs[12], vs[13]);
        draw_quad(vs[9], vs[10], vs[13], vs[14]);
        draw_quad(vs[10], vs[11], vs[14], vs[15]);
    }

    void draw_glyph(const Vec& pos, const Col& color, uint8_t c) {
        if (!m_active) return;
        Vec uv = { c % 16 * FONT_WIDTH, c / 16 * FONT_HEIGHT };
        Rect rect = { pos, pos + Vec(FONT_WIDTH, FONT_HEIGHT) };
        draw_rect(rect, color, uv);
    }
    void draw_text(const Vec& pos, const char* text) {
        if (!m_active) return;
        Vec p = pos;
        while (char c = *text++) {
            if (c == 10) {
                p.y += FONT_HEIGHT;
                p.x = pos.x;
                continue;
            }
            if (c > 32 || c < 128) draw_glyph(p, { 255, 255, 255, 255 }, c);
            p.x += FONT_WIDTH;
        }
    }

private:
    void draw_quad(const Vertex& v0,
                   const Vertex& v1,
                   const Vertex& v2,
                   const Vertex& v3)
    {
        m_vertices.emplace_back(v0);
        m_vertices.emplace_back(v1);
        m_vertices.emplace_back(v2);
        m_vertices.emplace_back(v2);
        m_vertices.emplace_back(v1);
        m_vertices.emplace_back(v3);
    }

    bool                m_active;
    std::vector<Vertex> m_vertices;
};


struct Window {
    const char* name;
    Rect        rect;

    // drawing
    bool        same_line;
    Rect        current_line;
    Rect        content_rect;
    DrawContext dc;
};

Vec                                  m_mouse_pos;
Vec                                  m_mouse_mov;
std::array<bool, 3>                  m_mouse_buttons;
std::array<bool, 3>                  m_mouse_buttons_clicked;
int                                  m_mouse_wheel;

const char*                          m_item_active;
const char*                          m_item_hovered;
const char*                          m_old_item_hovered;

Vec                                  m_window_spawn_pos = { 100, 100 };
std::vector<std::unique_ptr<Window>> m_windows;
std::vector<Window*>                 m_window_stack;
Window*                              m_window_active;
Window*                              m_window_hovered;

gfx::Texture2D*                      m_texture;
gfx::Shader*                         m_shader;
gfx::VertexArray*                    m_va;
gfx::VertexBuffer*                   m_vb;

std::array<char, 1024>               m_text_buffer;


void print_to_text_buffer(const char* fmt, va_list args) {
    vsnprintf(m_text_buffer.data(), m_text_buffer.size(), fmt, args);
}


void print_to_text_buffer(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    print_to_text_buffer(fmt, args);
    va_end(args);
}


Window* find_or_create_window(const char* name) {
    for (auto& w : m_windows) {
        if (strcmp(w->name, name) == 0) return w.get();
    }

    m_windows.push_back(std::make_unique<Window>());
    Window* w = m_windows.back().get();
    w->name = name;
    w->rect.min = w->rect.max = m_window_spawn_pos;

    m_window_spawn_pos += Vec(200, 0);

    return w;
}


void move_window_to_front(Window* w) {
    if (w != m_windows.back().get()) {
        for (auto it = m_windows.begin(); it != m_windows.end(); ++it) {
            if (it->get() == w) {
                std::rotate(it, it + 1, m_windows.end());
                break;
            }
        }
    }
}


Rect new_item_rect(Window* w, const Vec& size) {
    Vec pos;
    if (w->same_line) {
        w->same_line = false;
        pos = { w->current_line.max.x, w->current_line.min.y };
        short h = w->current_line.size().y;
        if (h > size.y) pos.y += (h - size.y) / 2;
        w->current_line.max = glm::max(w->current_line.max, pos + size);
    }
    else {
        pos = w->current_line.bl();
        w->current_line = { pos, pos + size };
    }
    Rect rect = { pos, pos + size };
    w->content_rect.max = glm::max(w->content_rect.max, rect.max);
    return rect;
}


Vec text_size(const char* text) {
    Vec s = { 0, FONT_HEIGHT };
    short x = 0;
    while (char c = *text++) {
        if (c == '\n') {
            s.y += FONT_HEIGHT;
            x = 0;
        }
        else {
            x += FONT_WIDTH;
            s.x = std::max(s.x, x);
        }
    }
    return s;
}


Col make_color(uint32_t c, uint8_t a = 255) {
    return { (c >> 16) & 255, (c >>  8) & 255, c & 255, a};
}


struct {
    Col window         = make_color(0x111111, 200);
    Col window_title   = make_color(0x000000, 100);
    Col button         = make_color(0x225577, 200);
    Col button_hovered = make_color(0x446688, 200);
    Col button_active  = make_color(0x447799, 200);
    Col frame          = make_color(0x225577, 100);
    Col frame_hovered  = make_color(0x446688, 100);
    Col frame_active   = make_color(0x447799, 100);
    Col handle         = make_color(0x447799, 200);
} const m_colors;


} // namespace


void init() {
    m_texture = load_texture("gui.png", gfx::FilterMode::Nearest);

    m_shader = gfx::Shader::create(
    R"(#version 100
    attribute vec2 a_pos;
    attribute vec2 a_uv;
    attribute vec4 a_col;
    varying vec2 v_uv;
    varying vec4 v_col;
    uniform vec2 scale;
    uniform vec2 texture_scale;
    void main() {
        v_uv = a_uv * texture_scale;
        v_col = a_col;
        gl_Position = vec4(vec2(2.0, -2.0) * scale * a_pos + vec2(-1.0, 1.0), 0.0, 1.0);
    })",
    R"(#version 100
    precision mediump float;
    uniform sampler2D texture;
    varying vec2 v_uv;
    varying vec4 v_col;
    void main() {
        gl_FragColor = v_col * texture2D(texture, v_uv);
    })");
    m_shader->set_uniform("texture", m_texture);
    m_shader->set_uniform("texture_scale",
            glm::vec2(1.0 / m_texture->width(), 1.0 / m_texture->height()));

    m_vb = gfx::VertexBuffer::create(gfx::BufferHint::StreamDraw);
    m_va = gfx::VertexArray::create();
    m_va->set_primitive_type(gfx::PrimitiveType::Triangles);
    m_va->set_attribute(0, m_vb, gfx::ComponentType::Int16, 2, false, 0, 12);
    m_va->set_attribute(1, m_vb, gfx::ComponentType::Int16, 2, false, 4, 12);
    m_va->set_attribute(2, m_vb, gfx::ComponentType::Uint8, 4, true,  8, 12);
}

void resize(int width, int height) {
    m_shader->set_uniform("scale", glm::vec2(1.0 / width, 1.0 / height));
}

void free() {
    delete m_texture;
    delete m_shader;
    delete m_va;
    delete m_vb;
    m_texture = nullptr;
    m_shader  = nullptr;
    m_va      = nullptr;
    m_vb      = nullptr;
}


//bool process_event(const SDL_Event& e) {
//    switch (e.type) {
//    case SDL_MOUSEWHEEL:
//        m_mouse_wheel += e.wheel.y;
//        return true;
//    default: return false;
//    }
//}


void new_frame() {
    // mouse
    int x, y;
//    Uint32 b = SDL_GetMouseState(&x, &y);
    x = y = 0;
    bool bs[3] = {
//        b & SDL_BUTTON(SDL_BUTTON_LEFT),
//        b & SDL_BUTTON(SDL_BUTTON_MIDDLE),
//        b & SDL_BUTTON(SDL_BUTTON_RIGHT),
    };
    for (int i = 0; i < 3; ++i) {
        m_mouse_buttons_clicked[i] = !m_mouse_buttons[i] && bs[i];
        m_mouse_buttons[i] = bs[i];
    }
    Vec p = { x, y };
    m_mouse_mov = p - m_mouse_pos;
    m_mouse_pos = p;

    // reset things
    if (!m_mouse_buttons[0]) {
        m_window_active = nullptr;
        m_item_active   = nullptr;
    }

    m_old_item_hovered = m_item_hovered;
    m_item_hovered     = nullptr;
    m_window_hovered   = nullptr;

    for (int i = (int) m_windows.size() - 1; i >= 0; --i) {
        auto& w = m_windows[i];
        w->dc.clear();
        w->dc.set_active(w->content_rect.size() != Vec(0, 0));

        if (!m_window_hovered && w->rect.contains(m_mouse_pos)) m_window_hovered = w.get();

        // adjust window size
        if (w->content_rect.min != w->content_rect.max) {
            w->rect.max = glm::max(w->rect.max, w->content_rect.max + Vec(4));
        }
    }

    // have a default debug window
    begin_window("Debug");
}


void render() {
    end_window();

    m_mouse_wheel = 0;

    gfx::RenderState rs;
    rs.blend_enabled = true;
    rs.blend_func_src_rgb = gfx::BlendFunc::SrcAlpha;
    rs.blend_func_dst_rgb = gfx::BlendFunc::OneMinusSrcAlpha;
    rs.cull_face_enabled = false;


    // render each window separately
    for (auto& w : m_windows) {
        auto& vs = w->dc.vertices();
        if (vs.empty()) continue;
        m_vb->init_data(vs);
        m_va->set_count(vs.size());
        gfx::screen()->draw(rs, m_shader, m_va);
    }
}


void set_next_window_pos(const Vec& pos) {
    m_window_spawn_pos = pos;
}


void begin_window(const char* name) {
    Window* w = find_or_create_window(name);
    m_window_stack.emplace_back(w);

    // have we been here before in this frame?
    if (!w->dc.vertices().empty()) return;

    bool hovered = w == m_window_hovered;
    bool clicked = hovered && m_mouse_buttons_clicked[0] && !m_old_item_hovered;
    if (clicked) {
        m_window_active = w;
        move_window_to_front(w);
    }
    bool active = w == m_window_active;
    if (active) {
        w->rect.min += m_mouse_mov;
        w->rect.max += m_mouse_mov;
    }

    Rect title_rect  = { w->rect.min, w->rect.min + text_size(name) + Vec(12) };
    w->rect.max      = glm::max(w->rect.max, title_rect.max);
    title_rect.max.x = w->rect.max.x;

    w->dc.draw_rect(w->rect, m_colors.window, RECT_FILL_ROUND_3);
    w->dc.draw_rect(title_rect, m_colors.window_title, RECT_FILL_ROUND_3);
    w->dc.draw_text(title_rect.min + Vec(6), name);

    Vec p = title_rect.bl() + Vec(4);
    w->content_rect = { p, p };
    w->current_line = { p, p };
}


void end_window() {
    m_window_stack.pop_back();
}


void same_line(short offset) {
    Window* w = m_window_stack.back();
    w->same_line = true;
    w->current_line.max.x = std::max<short>(w->current_line.max.x,
                                            w->current_line.min.x + offset);
}


void separator() {
    Window* w = m_window_stack.back();
    Vec size;
    bool sl = w->same_line;
    if (sl) size = Vec(5, w->current_line.size().y);
    else size = Vec(w->rect.size().x - 8, 5);
    w->dc.draw_rect(new_item_rect(w, size).expand(-2), { 255, 255, 255, 50 }, RECT_FILL);
    if (sl) same_line();
}


void text(const char* fmt, ...) {
    Window* w = m_window_stack.back();

    va_list args;
    va_start(args, fmt);
    print_to_text_buffer(fmt, args);
    va_end(args);

    Rect rect = new_item_rect(w, text_size(m_text_buffer.data()) + Vec(4));

    w->dc.draw_text(rect.min + Vec(2), m_text_buffer.data());
}


bool button(const char* label) {
    Window* w = m_window_stack.back();

    Rect rect = new_item_rect(w, text_size(label) + Vec(12));

    Rect bb = rect.expand(-2);
    bool hovered = w == m_window_hovered && bb.contains(m_mouse_pos);
    if (hovered) m_item_hovered = label;
    bool clicked = hovered && m_mouse_buttons_clicked[0];
    if (clicked) {
        m_item_active = label;
        move_window_to_front(w);
    }
    bool active = m_item_active == label;

    Col color = active  ? m_colors.button_active :
                hovered ? m_colors.button_hovered :
                          m_colors.button;

    w->dc.draw_rect(bb, color, RECT_FILL_ROUND_1);
    w->dc.draw_text(bb.min + Vec(4), label);

    return clicked;
}


bool checkbox(const char* label, bool& v) {
    Window* w = m_window_stack.back();

    Vec s = text_size(label);

    float check_width = s.y + 12;
    float label_width = s.x + 4;

    Rect rect = new_item_rect(w, Vec(check_width + label_width, check_width));

    Rect bb = Rect(rect.min, rect.min + Vec(check_width)).expand(-2);

    bool hovered = w == m_window_hovered && rect.expand(-2).contains(m_mouse_pos);
    if (hovered) m_item_hovered = label;
    bool clicked = hovered && m_mouse_buttons_clicked[0];
    if (clicked) {
        m_item_active = label;
        move_window_to_front(w);
        v = !v;
    }
    bool active = m_item_active == label;

    // draw check
    {
        Col color = active  ? m_colors.frame_active :
                    hovered ? m_colors.frame_hovered :
                              m_colors.frame;
        w->dc.draw_rect(bb, color, RECT_FILL_ROUND_1);

        if (v) {
            w->dc.draw_rect(bb.expand(-6), m_colors.handle, RECT_FILL);
        }
    }

    // draw label
    {
        w->dc.draw_text(rect.min + Vec(check_width + 2, 6), label);
    }

    return clicked;
}


bool radio_button(const char* label, int& v, int value) {
    Window* w = m_window_stack.back();

    Vec s = text_size(label);

    float check_width = s.y + 12;
    float label_width = s.x + 4;

    Rect rect = new_item_rect(w, Vec(check_width + label_width, check_width));

    Rect bb = Rect(rect.min, rect.min + Vec(check_width)).expand(-2);

    bool hovered = w == m_window_hovered && rect.expand(-2).contains(m_mouse_pos);
    if (hovered) m_item_hovered = label;
    bool clicked = hovered && m_mouse_buttons_clicked[0];
    bool changed = false;
    if (clicked) {
        m_item_active = label;
        move_window_to_front(w);
        changed = v != value;
        v = value;
    }
    bool active = m_item_active == label;

    // draw check
    {
        Col color = active  ? m_colors.frame_active :
                    hovered ? m_colors.frame_hovered :
                              m_colors.frame;
        w->dc.draw_rect(bb, color, RECT_FILL_ROUND_1);

        if (v == value) {
            w->dc.draw_rect(bb.expand(-6), m_colors.handle, RECT_FILL);
        }
    }

    // draw label
    {
        w->dc.draw_text(rect.min + Vec(check_width + 2, 6), label);
    }

    return changed;
}


const int item_width_default = 24 * FONT_WIDTH;


bool drag_float(const char* label, float& v, float speed, float min, float max, const char* fmt) {
    Window* w = m_window_stack.back();

    Vec s = text_size(label);
    Rect rect = new_item_rect(w, Vec(item_width_default + 12 + s.x + 4, s.y + 12));

    Rect drag_rect  = { rect.min, rect.min + Vec(item_width_default + 12, s.y + 12) };
    Rect bb = drag_rect.expand(-2);

    bool hovered = w == m_window_hovered && bb.contains(m_mouse_pos);
    if (hovered) m_item_hovered = label;
    bool clicked = hovered && m_mouse_buttons_clicked[0];
    if (clicked) {
        m_item_active = label;
        move_window_to_front(w);
    }
    bool active = m_item_active == label;
    float old_v = v;
    if (active || (hovered && m_mouse_wheel != 0)) {
        if (min < max) {
            // no need for speed
            v +=  (m_mouse_mov.x + m_mouse_wheel) * (max - min) / (bb.size().x - 4) * 0.5;
            v = glm::clamp(v, min, max);
        }
        else {
            v += (m_mouse_mov.x + m_mouse_wheel) * speed;
        }
    }
    bool changed = v != old_v;

    // draw item
    {
        Col color = active  ? m_colors.frame_active :
                    hovered ? m_colors.frame_hovered :
                              m_colors.frame;

        w->dc.draw_rect(bb, color, RECT_FILL_ROUND_1);

        // handle
        if (min < max) {
            short x = (bb.size().x - 4) * (v - min) / (max - min);
            Rect handle_rect = { Vec(bb.min.x + x, bb.min.y),
                                 Vec(bb.min.x + x + 4, bb.max.y) };
            w->dc.draw_rect(handle_rect, m_colors.handle);
        }

        print_to_text_buffer(fmt, v);
        Vec s = text_size(m_text_buffer.data());
        w->dc.draw_text(bb.center() - Vec(s.x / 2, s.y / 2), m_text_buffer.data());
    }

    // draw label
    {
        w->dc.draw_text(drag_rect.tr() + Vec(2, 6), label);
    }

    return changed;
}


} // namespace
