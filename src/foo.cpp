#include "foo.hpp"
#include "gfx.hpp"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_FAILURE_STRINGS
#define STBI_ONLY_PNG
#include "stb_image.h"


#ifdef ANDROID
#include <android/asset_manager.h>
extern AAssetManager* g_asset_manager;
bool load_asset(char const* name, std::vector<uint8_t>& buf) {
    AAsset* ad = AAssetManager_open(g_asset_manager, name, AASSET_MODE_BUFFER);
    if (!ad) {
        LOGE("could not open asset %s", name);
        return false;
    }
    buf.resize(AAsset_getLength(ad));
    int len = AAsset_read(ad, buf.data(), buf.size());
    AAsset_close(ad);
    if (len != buf.size()) {
        LOGE("could not open asset %s", name);
        return false;
    }
    return true;
}
#else
#include <fstream>
#include <string>
bool load_asset(char const* name, std::vector<uint8_t>& buf) {
    std::ifstream f("assets/" + std::string(name), std::ios::in | std::ios::binary);
    if (!f.is_open()) {
        LOGE("could not open asset %s", name);
        return false;
    }
    buf = std::vector<uint8_t>((std::istreambuf_iterator<char>(f)),
                                std::istreambuf_iterator<char>());
    return true;
}
#endif


gfx::Texture2D* load_texture(char const*     name,
                             gfx::FilterMode filter,
                             gfx::WrapMode   wrap)
{
    std::vector<uint8_t> buf;
    if (!load_asset(name, buf)) return nullptr;
    int w, h, c;
    uint8_t* p = stbi_load_from_memory(buf.data(), buf.size(), &w, &h, &c, 0);
    gfx::TextureFormat f = c == 1 ? gfx::TextureFormat::Alpha
                         : c == 3 ? gfx::TextureFormat::RGB
                                  : gfx::TextureFormat::RGBA;
    gfx::Texture2D* t = gfx::Texture2D::create(f, w, h, p, filter, wrap);
    stbi_image_free(p);
    return t;
}
