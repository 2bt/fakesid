#pragma once
#include "gfx.hpp"


using Color = glm::u8vec4;
using Vec   = glm::i16vec2;


namespace render {


    struct Vertex {
        Vec   pos;
        Vec   uv;
        Color col;
    };


    class DrawContext {
    public:

        std::vector<Vertex> const& vertices() const { return m_vertices; }

        void clear() { m_vertices.clear(); }

        void copy(Vec const& pos,
                  Vec const& size,
                  Vec const& uv,
                  Color c = {255, 255, 255, 255})
        {
            copy(pos, size, uv, size, c);
        }

        void copy(Vec const& pos,
                  Vec const& size,
                  Vec const& uv,
                  Vec const& uv_size,
                  Color c = {255, 255, 255, 255});

        void quad(Vertex const& v0,
                  Vertex const& v1,
                  Vertex const& v2,
                  Vertex const& v3);

    private:
        std::vector<Vertex> m_vertices;
    };


    void init();
    void free();
    void draw(gfx::RenderTarget* rt,
              DrawContext const& dc,
              gfx::Texture2D*    texture);

} // namespace
