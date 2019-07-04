#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>


struct SDL_Surface;


namespace gfx {


enum class DepthTestFunc { Never, Less, Equal, LEqual };


enum class CullFace { Front, Back, BackAndFront };


enum class BlendFunc {
    Zero, One, SrcColor, OneMinusSrcColor, DstColor, OneMinusDstColor,
    SrcAlpha, OneMinusSrcAlpha, DstAlpha, OneMinusDstAlpha,
    ConstantColor, OneMinusConstantColor,
    ConstantAlpha, OneMinusConstantAlph,
    SrcAlphaSaturate,
};


enum class BlendEquation { Add, Subtract, ReverseSubtract };


struct Rect {
    int x, y, w, h;
    friend bool operator==(Rect const& a, Rect const& b) {
        return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
    }
    friend bool operator!=(Rect const& a, Rect const& b) {
        return !(a == b);
    }
};


struct RenderState {
    Rect          viewport             = { 0, 0, 0, 0 };

    // depth
    bool          depth_test_enabled   = false;
    DepthTestFunc depth_test_func      = DepthTestFunc::LEqual;

    // cull face
    bool          cull_face_enabled    = true;
    CullFace      cull_face            = CullFace::Back;

    // scissor
    bool          scissor_test_enabled = false;
    Rect          scissor_box          = { 0, 0, 0, 0 };

    // blend
    bool          blend_enabled        = false;
    BlendFunc     blend_func_src_rgb   = BlendFunc::One;
    BlendFunc     blend_func_src_alpha = BlendFunc::Zero;
    BlendFunc     blend_func_dst_rgb   = BlendFunc::One;
    BlendFunc     blend_func_dst_alpha = BlendFunc::Zero;
    BlendEquation blend_equation_rgb   = BlendEquation::Add;
    BlendEquation blend_equation_alpha = BlendEquation::Add;
    glm::vec4     blend_color          = { 0, 0, 0, 0 };

    float         line_width           = 1;
};


enum class BufferHint { StaticDraw, StreamDraw, DynamicDraw };


struct VertexBuffer {
    static VertexBuffer* create(BufferHint hint);
    virtual ~VertexBuffer() {}
    virtual void init_data(const void* data, int size) = 0;
    template<class T>
    void init_data(const std::vector<T>& data) {
        init_data(static_cast<const void*>(data.data()), data.size() * sizeof(T));
    }
};


struct IndexBuffer {
    static IndexBuffer* create(BufferHint hint);
    virtual ~IndexBuffer() {}
    virtual void init_data(const void* data, int size) = 0;
    void init_data(const std::vector<int>& data) {
        init_data(static_cast<const void*>(data.data()), data.size() * sizeof(int));
    }
};


enum class PrimitiveType { Points, LineStrip, LineLoop, Lines, TriangleStrip, TriangleFan, Triangles };


enum class ComponentType { Int8, Uint8, Int16, Uint16, Int32, Uint32, Float };


struct VertexArray {
    static VertexArray* create();
    virtual ~VertexArray() {}
    virtual void set_first(int i) = 0;
    virtual void set_count(int i) = 0;
    virtual void set_primitive_type(PrimitiveType t) = 0;
    virtual void set_attribute(int i, VertexBuffer* vb, ComponentType component_type, int component_count,
                               bool normalized, int offset, int stride) = 0;
    virtual void set_attribute(int i, float f) = 0;
    virtual void set_attribute(int i, glm::vec2 const& v) = 0;
    virtual void set_attribute(int i, glm::vec3 const& v) = 0;
    virtual void set_attribute(int i, glm::vec4 const& v) = 0;

    virtual void set_index_buffer(IndexBuffer* ib) = 0;

    virtual PrimitiveType get_primitive_type() const = 0;
};


enum class WrapMode { Clamp, Repeat, MirrowedRepeat };

enum class FilterMode { Nearest, Linear, Trilinear };

enum class TextureFormat { Alpha, RGB, RGBA, Depth };

struct Texture2D {
    static Texture2D* create(TextureFormat format, int w, int h, void const* data = nullptr,
                             FilterMode filter = FilterMode::Nearest, WrapMode wrap = WrapMode::Clamp);

    virtual ~Texture2D() {}
    virtual int get_width() const = 0;
    virtual int get_height() const = 0;
};



struct Shader {
    static Shader* create(const char* vs, const char* fs);
    virtual ~Shader() {}
    virtual bool has_uniform(std::string const& name) = 0;
    virtual void set_uniform(std::string const& name, Texture2D* v) = 0;
    virtual void set_uniform(std::string const& name, int v) = 0;
    virtual void set_uniform(std::string const& name, float v) = 0;
    virtual void set_uniform(std::string const& name, glm::vec2 const& v) = 0;
    virtual void set_uniform(std::string const& name, glm::vec3 const& v) = 0;
    virtual void set_uniform(std::string const& name, glm::vec4 const& v) = 0;
    virtual void set_uniform(std::string const& name, glm::mat3 const& v) = 0;
    virtual void set_uniform(std::string const& name, glm::mat4 const& v) = 0;
};


struct Framebuffer {
    static Framebuffer* create();
    virtual ~Framebuffer() {}
    virtual void attach_color(Texture2D* t) = 0;
    virtual void attach_depth(Texture2D* t) = 0;
    virtual bool is_complete() const = 0;

    virtual void clear(const glm::vec4& color) = 0;
    virtual void draw(const RenderState& rs, Shader* shader, VertexArray* va) = 0;
};


void clear(const glm::vec4& color);
void draw(const RenderState& rs, Shader* shader, VertexArray* va);
void resize(int width, int height);
void init();

} // namespace
