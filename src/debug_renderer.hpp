// vim: et ts=4 sts=4 sw=4
#pragma once
#include "gfx.hpp"
#include <array>


class DebugRenderer {
public:

    void init() {
        m_shader = gfx::Shader::create(
            R"(
                #version 100
                attribute vec2 in_pos;
                attribute vec4 in_color;
                attribute float in_point_size;
                uniform vec2 resolution;
                varying vec4 ex_color;
                void main() {
                    gl_Position = vec4(in_pos.x, -in_pos.y, 0.0, 1.0);
                    ex_color = in_color;
                    gl_PointSize = in_point_size;
                })",
            R"(
                #version 100
                precision mediump float;
                varying vec4 ex_color;
                void main() {
                    gl_FragColor = ex_color;
                })");

        m_rs.line_width           = 1;
        m_rs.cull_face_enabled    = false;
        m_rs.blend_enabled        = true;
        m_rs.blend_func_src_rgb   = gfx::BlendFunc::SrcAlpha;
        m_rs.blend_func_src_alpha = gfx::BlendFunc::SrcAlpha;
        m_rs.blend_func_dst_rgb   = gfx::BlendFunc::OneMinusSrcAlpha;
        m_rs.blend_func_dst_alpha = gfx::BlendFunc::OneMinusSrcAlpha;

        m_vb = gfx::VertexBuffer::create(gfx::BufferHint::StreamDraw);
        m_va = gfx::VertexArray::create();
        m_va->set_attribute(0, m_vb, gfx::ComponentType::Float, 2, false,  0, sizeof(Vert));
        m_va->set_attribute(1, m_vb, gfx::ComponentType::Uint8, 4, true,   8, sizeof(Vert));
        m_va->set_attribute(2, m_vb, gfx::ComponentType::Float, 1, false, 12, sizeof(Vert));
    }

    void free() {
        delete m_va;
        delete m_vb;
        delete m_shader;
        m_va     = nullptr;
        m_vb     = nullptr;
        m_shader = nullptr;
    }


    void push() {
        assert(m_transform_index < m_transforms.size() - 2);
        ++m_transform_index;
        transform() = m_transforms[m_transform_index - 1];
    }
    void pop() {
        assert(m_transform_index > 0);
        --m_transform_index;
    }
    void origin() {
        transform() = { 1, 0, 0, 1, 0, 0 };
    }
    void scale(const glm::vec2& v) { scale(v.x, v.y); }
    void scale(float x) { scale(x, x); }
    void scale(float x, float y) {
        transform()[0][0] *= x;
        transform()[1][1] *= y;
    }
    void translate(const glm::vec2& v) { translate(v.x, v.y); }
    void translate(float x, float y) {
        transform()[2][0] += x * transform()[0][0];
        transform()[2][1] += y * transform()[1][1];
    }

    void set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a=255) {
        m_color.r = r;
        m_color.g = g;
        m_color.b = b;
        m_color.a = a;
    }
    void set_point_size(float s) {
        m_point_size = s;
    }
    void set_line_width(float w) {
        if (w != m_rs.line_width &&
            m_va->get_primitive_type() == gfx::PrimitiveType::Lines) flush();
        m_rs.line_width = w;
    }

    void point(float x, float y) {
        point(glm::vec2(x, y));
    }
    void point(const glm::vec2& p) {
        if (m_va->get_primitive_type() != gfx::PrimitiveType::Points) {
            flush();
            m_va->set_primitive_type(gfx::PrimitiveType::Points);
        }
        m_verts.emplace_back(transform() * glm::vec3(p, 1), m_color, m_point_size);
    }
    void line(float x1, float y1, float x2, float y2) {
        line(glm::vec2(x1, y1), glm::vec2(x2, y2));
    }
    void line(const glm::vec2& p1, const glm::vec2& p2) {
        if (m_va->get_primitive_type() != gfx::PrimitiveType::Lines) {
            flush();
            m_va->set_primitive_type(gfx::PrimitiveType::Lines);
        }
        m_verts.emplace_back(transform() * glm::vec3(p1, 1), m_color);
        m_verts.emplace_back(transform() * glm::vec3(p2, 1), m_color);
    }
    void rect(const glm::vec2& p1, const glm::vec2& p2) {
        line(p1.x, p1.y, p1.x, p2.y);
        line(p1.x, p2.y, p2.x, p2.y);
        line(p2.x, p2.y, p2.x, p1.y);
        line(p2.x, p1.y, p1.x, p1.y);
    }
    void filled_rect(const glm::vec2& p1, const glm::vec2& p2) {
        triangle(p1, p2, {p1.x, p2.y});
        triangle(p1, {p2.x, p1.y}, p2);
    }

    void filled_polygon(glm::vec2 const* poly, int len) {
        for (int i = 2; i < len; ++i) {
            triangle(poly[0], poly[i - 1], poly[i]);
        }
    }
    template<class T>
    void filled_polygon(T const& poly) {
        filled_polygon(poly.data(), poly.size());
    }

    void polygon(glm::vec2 const* poly, int len) {
        for (int i = 0; i < len; ++i) {
            line(poly[i], poly[(i + 1) % len]);
        }
    }
    template<class T>
    void polygon(T const& poly) {
        polygon(poly.data(), poly.size());
    }

    void triangle(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3) {
        if (m_va->get_primitive_type() != gfx::PrimitiveType::Triangles) {
            flush();
            m_va->set_primitive_type(gfx::PrimitiveType::Triangles);
        }
        m_verts.emplace_back(transform() * glm::vec3(p1, 1), m_color);
        m_verts.emplace_back(transform() * glm::vec3(p2, 1), m_color);
        m_verts.emplace_back(transform() * glm::vec3(p3, 1), m_color);
    }

    void flush() {
        m_vb->init_data(m_verts);
        m_va->set_count(m_verts.size());
        m_verts.clear();
        gfx::draw(m_rs, m_shader, m_va);
    }

    glm::mat3x2& transform() { return m_transforms[m_transform_index]; }


private:


    struct Vert {
        glm::vec2   pos;
        glm::u8vec4 color;
        float       size;
        Vert(const glm::vec2& p, const glm::u8vec4& c, float s=0)
            : pos(p), color(c), size(s) {}
    };

    glm::u8vec4                m_color;
    float                      m_point_size = 1;

    std::vector<Vert>          m_verts;

    gfx::RenderState           m_rs;
    gfx::Shader*               m_shader;
    gfx::VertexArray*          m_va;
    gfx::VertexBuffer*         m_vb;

    std::array<glm::mat3x2, 4> m_transforms;
    size_t                     m_transform_index = 0;
};

