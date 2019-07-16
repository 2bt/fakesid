#include "foo.hpp"
#include "gfx.hpp"
#include "player.hpp"
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_FAILURE_STRINGS
#define STBI_ONLY_PNG
#include "stb_image.h"


#ifdef ANDROID
#include <android/asset_manager.h>
#include <jni.h>
#include <oboe/Oboe.h>


extern AAssetManager* g_asset_manager;
extern JNIEnv*        g_env;


namespace android {
namespace {


class : public oboe::AudioStreamCallback {
public:
    oboe::DataCallbackResult onAudioReady(
            oboe::AudioStream *oboeStream,
            void              *audioData,
            int32_t           numFrames) override
    {
        player::fill_buffer((short*) audioData, numFrames);
        return oboe::DataCallbackResult::Continue;
    }
} s_audio_callback;


oboe::AudioStream* s_stream;


} // namespace


bool start_audio() {
    if (s_stream) stop_audio();

    oboe::AudioStreamBuilder builder;
    builder.setDirection(oboe::Direction::Output);
    builder.setSampleRate(MIXRATE);
    builder.setFormat(oboe::AudioFormat::I16);
    builder.setChannelCount(oboe::ChannelCount::Mono);
    builder.setCallback(&s_audio_callback);
    builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
    builder.setSharingMode(oboe::SharingMode::Exclusive);

    oboe::Result result = builder.openStream(&s_stream);
    if (result != oboe::Result::OK) {
        LOGE("openStream: %s", oboe::convertToText(result));
        return false;
    }

//    s_stream->setBufferSizeInFrames(s_stream->getFramesPerBurst() * 2);

    auto rate = s_stream->getSampleRate();
    if (rate != MIXRATE) {
        LOGW("mixrate is %d but should be %d", rate, MIXRATE);
    }

    result = s_stream->requestStart();
    if (result != oboe::Result::OK) {
        LOGE("result is not okay: %s", oboe::convertToText(result));
        return false;
    }
    return true;
}

void stop_audio() {
    if (!s_stream) return;
    s_stream->close();
    delete s_stream;
    s_stream = nullptr;
}

bool load_asset(std::string const& name, std::vector<uint8_t>& buf) {
    AAsset* ad = AAssetManager_open(g_asset_manager, name.c_str(), AASSET_MODE_BUFFER);
    if (!ad) {
        LOGE("could not open asset %s", name.c_str());
        return false;
    }
    buf.resize(AAsset_getLength(ad));
    int len = AAsset_read(ad, buf.data(), buf.size());
    AAsset_close(ad);
    if (len != buf.size()) {
        LOGE("could not open asset %s", name.c_str());
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
    jclass clazz = g_env->FindClass("com/twobit/fakesid/MainActivity");
    jmethodID method = g_env->GetStaticMethodID(clazz, "showKeyboard", "()V");
    g_env->CallStaticVoidMethod(clazz, method);
}

void hide_keyboard() {
    jclass clazz = g_env->FindClass("com/twobit/fakesid/MainActivity");
    jmethodID method = g_env->GetStaticMethodID(clazz, "hideKeyboard", "()V");
    g_env->CallStaticVoidMethod(clazz, method);
}

#else
#include <fstream>
#include <SDL2/SDL.h>


namespace android {


static void audio_callback(void* userdata, Uint8* stream, int len) {
    player::fill_buffer((short*) stream, len / 2);
}

bool start_audio() {
    SDL_AudioSpec spec = { MIXRATE, AUDIO_S16, 1, 0, SAMPLES_PER_FRAME, 0, 0, audio_callback };
    SDL_OpenAudio(&spec, nullptr);
    SDL_PauseAudio(0);
    return true;
}

void stop_audio() {
    SDL_CloseAudio();
}


bool load_asset(std::string const& name, std::vector<uint8_t>& buf) {
    std::ifstream f("assets/" + name, std::ios::in | std::ios::binary);
    if (!f.is_open()) {
        LOGE("could not open asset %s", name.c_str());
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
