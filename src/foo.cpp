#include "foo.hpp"
#include "gfx.hpp"
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_FAILURE_STRINGS
#define STBI_ONLY_PNG
#include "stb_image.h"


#ifdef ANDROID
#include <android/asset_manager.h>
#include <jni.h>
//#include <sys/stat.h>


extern AAssetManager* g_asset_manager;
extern JNIEnv*        g_env;


namespace android {

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


std::string get_storage_dir() {

    jclass j_env_class = g_env->FindClass("android/os/Environment");
    jmethodID j_mid = g_env->GetStaticMethodID(j_env_class, "getExternalStorageDirectory",  "()Ljava/io/File;");
    jobject j_file = g_env->CallStaticObjectMethod(j_env_class, j_mid);
    if (!j_file) return "";

    j_mid = g_env->GetMethodID(g_env->GetObjectClass(j_file), "mkdirs", "()Z");
    g_env->CallBooleanMethod(j_file, j_mid);

    j_mid = g_env->GetMethodID(g_env->GetObjectClass(j_file), "getAbsolutePath", "()Ljava/lang/String;");
    jstring j_path = (jstring) g_env->CallObjectMethod(j_file, j_mid);
    const char* str = g_env->GetStringUTFChars(j_path, nullptr);
    std::string root_dir = str;
    g_env->ReleaseStringUTFChars(j_path, str);

    return root_dir + "/fakesid";
}


void show_keyboard() {
    jclass clazz = g_env->FindClass("com/twobit/fakesid/Activity");
    jmethodID method = g_env->GetStaticMethodID(clazz, "showKeyboard", "()V");
    g_env->CallStaticVoidMethod(clazz, method);
}

void hide_keyboard() {
    jclass clazz = g_env->FindClass("com/twobit/fakesid/Activity");
    jmethodID method = g_env->GetStaticMethodID(clazz, "hideKeyboard", "()V");
    g_env->CallStaticVoidMethod(clazz, method);
}

#else
#include <fstream>


namespace android {


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



std::string get_storage_dir() { return "."; }


void show_keyboard() {}


void hide_keyboard() {}


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


} // namespace
