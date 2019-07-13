#include "render.hpp"

namespace render {
namespace {


gfx::RenderState   m_rs;
gfx::Shader*       m_shader;
gfx::VertexArray*  m_va;
gfx::VertexBuffer* m_vb;


} // namespace


void DrawContext::copy(Vec const& pos, Vec const& size, Vec const& uv, Vec const& uv_size, Color c) {
    Vertex vs[] = {
        { pos, uv, c},
        { pos + Vec(0, size.y), uv + Vec(0, uv_size.y), c},
        { pos + Vec(size.x, 0), uv + Vec(uv_size.x, 0), c},
        { pos + size, uv + uv_size, c},
    };
    quad(vs[0], vs[1], vs[2], vs[3]);
}

void DrawContext::quad(Vertex const& v0, Vertex const& v1, Vertex const& v2, Vertex const& v3) {
    m_vertices.emplace_back(v0);
    m_vertices.emplace_back(v1);
    m_vertices.emplace_back(v2);
    m_vertices.emplace_back(v2);
    m_vertices.emplace_back(v1);
    m_vertices.emplace_back(v3);
}


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
    uniform bool flip;
    void main() {
//        v_uv = a_uv;
        v_uv = a_uv * texture_scale;
        v_col = a_col;
        gl_Position = vec4(vec2(screen_scale) * a_pos - vec2(1.0), 0.0, 1.0);
        if (flip) gl_Position.y = -gl_Position.y;
    })",
    R"(#version 100
    precision mediump float;
    uniform sampler2D texture;
    varying vec2 v_uv;
    varying vec4 v_col;
    void main() {
//        vec2 f = fract(v_uv);
//        f = max(min(f * 3.0, 0.5), 1.0 - (vec2(1.0) - f) * 3.0);
//        gl_FragColor = v_col * texture2D(texture, (floor(v_uv) + f) * texture_scale);
        gl_FragColor = v_col * texture2D(texture, v_uv);
    })");
    m_vb = gfx::VertexBuffer::create(gfx::BufferHint::StreamDraw);
    m_va = gfx::VertexArray::create();
    m_va->set_primitive_type(gfx::PrimitiveType::Triangles);
    m_va->set_attribute(0, m_vb, gfx::ComponentType::Int16, 2, false, 0, 12);
    m_va->set_attribute(1, m_vb, gfx::ComponentType::Int16, 2, false, 4, 12);
    m_va->set_attribute(2, m_vb, gfx::ComponentType::Uint8, 4, true,  8, 12);

    m_rs.blend_enabled      = true;
    m_rs.blend_func_src_rgb   = gfx::BlendFunc::SrcAlpha;
    m_rs.blend_func_src_alpha = gfx::BlendFunc::SrcAlpha;
    m_rs.blend_func_dst_rgb   = gfx::BlendFunc::OneMinusSrcAlpha;
    m_rs.blend_func_dst_alpha = gfx::BlendFunc::OneMinusSrcAlpha;
    m_rs.cull_face_enabled    = false;
}

void free() {
    delete m_shader;
    delete m_va;
    delete m_vb;
    m_shader = nullptr;
    m_va     = nullptr;
    m_vb     = nullptr;
}

void draw(gfx::RenderTarget* rt, std::vector<Vertex> const& vertice, gfx::Texture2D* texture) {
    m_shader->set_uniform("screen_scale", 2.0f / glm::vec2(rt->width(), rt->height()));
    m_shader->set_uniform("texture_scale", 1.0f / glm::vec2(texture->width(), texture->height()));
    m_shader->set_uniform("texture", texture);
    m_shader->set_uniform("flip", dynamic_cast<gfx::Screen*>(rt) != nullptr);

    m_vb->init_data(vertice);
    m_va->set_count(vertice.size());
    rt->draw(m_rs, m_shader, m_va);
}


} // namespace
