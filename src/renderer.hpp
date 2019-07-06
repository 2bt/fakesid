#include "gfx.hpp"

using Color = glm::u8vec4;
using Vec   = glm::i16vec2;


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


struct Vertex {
    Vec   pos;
    Vec   uv;
    Color col;
};


class DrawContext {
public:

    std::vector<Vertex> const& vertices() const { return m_vertices; }

    void clear() { m_vertices.clear(); }

    void rect(Rect const& r, Color c) {
        Vertex vs[] = {
            { r.tl(), {0, 0}, c},
            { r.bl(), {0, 1}, c},
            { r.tr(), {1, 0}, c},
            { r.br(), {1, 1}, c},
        };
        quad(vs[0], vs[1], vs[2], vs[3]);
    }

    void rect(Rect const& r, Color c, Vec const& uv) {
        Vec s = r.size();
        Vertex vs[] = {
            { r.tl(), uv, c },
            { r.bl(), uv + Vec(0, s.y), c },
            { r.tr(), uv + Vec(s.x, 0), c },
            { r.br(), uv + s, c },
        };
        quad(vs[0], vs[1], vs[2], vs[3]);
    }

    void quad(Vertex const& v0,
              Vertex const& v1,
              Vertex const& v2,
              Vertex const& v3)
    {
        m_vertices.emplace_back(v0);
        m_vertices.emplace_back(v1);
        m_vertices.emplace_back(v2);
        m_vertices.emplace_back(v2);
        m_vertices.emplace_back(v1);
        m_vertices.emplace_back(v3);
    }

private:
    std::vector<Vertex> m_vertices;
};


class Renderer {
public:
    void init() {
        m_shader = gfx::Shader::create(
        R"(#version 100
        attribute vec2 a_pos;
        attribute vec2 a_uv;
        attribute vec4 a_col;
        varying vec2 v_uv;
        varying vec4 v_col;
        uniform vec2 screen_scale;
        uniform vec2 texture_scale;
        void main() {
            v_uv = a_uv * texture_scale;
            v_col = a_col;
            gl_Position = vec4(vec2(2.0, -2.0) * screen_scale * a_pos + vec2(-1.0, 1.0), 0.0, 1.0);
        })",
        R"(#version 100
        precision mediump float;
        uniform sampler2D texture;
        varying vec2 v_uv;
        varying vec4 v_col;
        void main() {
            gl_FragColor = v_col * vec4(1.0, 1.0, 1.0, texture2D(texture, v_uv).a);
        })");
        m_vb = gfx::VertexBuffer::create(gfx::BufferHint::StreamDraw);
        m_va = gfx::VertexArray::create();
        m_va->set_primitive_type(gfx::PrimitiveType::Triangles);
        m_va->set_attribute(0, m_vb, gfx::ComponentType::Int16, 2, false, 0, 12);
        m_va->set_attribute(1, m_vb, gfx::ComponentType::Int16, 2, false, 4, 12);
        m_va->set_attribute(2, m_vb, gfx::ComponentType::Uint8, 4, true,  8, 12);

        m_rs.blend_enabled      = true;
        m_rs.blend_func_src_rgb = gfx::BlendFunc::SrcAlpha;
        m_rs.blend_func_dst_rgb = gfx::BlendFunc::OneMinusSrcAlpha;
        m_rs.cull_face_enabled  = false;
    }

    void free() {
        delete m_shader;
        delete m_va;
        delete m_vb;
        m_shader  = nullptr;
        m_va      = nullptr;
        m_vb      = nullptr;
    }

    void draw(gfx::RenderTarget* rt, std::vector<Vertex> const& vertice, gfx::Texture2D* texture) {
        m_shader->set_uniform("screen_scale", glm::vec2(1.0 / rt->width(), 1.0 / rt->height()));
        m_shader->set_uniform("texture", texture);
        m_shader->set_uniform("texture_scale", glm::vec2(1.0 / texture->width(), 1.0 / texture->height()));
        m_vb->init_data(vertice);
        m_va->set_count(vertice.size());
        rt->draw(m_rs, m_shader, m_va);
    }

private:
    gfx::RenderState   m_rs;
    gfx::Shader*       m_shader;
    gfx::VertexArray*  m_va;
    gfx::VertexBuffer* m_vb;

};

