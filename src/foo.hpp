#pragma once
#include "gfx.hpp"

#ifdef ANDROID
#include <android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO,  "FOOBAR", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN,  "FOOBAR", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "FOOBAR", __VA_ARGS__))
#else
#include <cstdio>
#define LOGI(fmt, ...) printf(            fmt "\n", ##__VA_ARGS__)
#define LOGW(fmt, ...) printf("warning: " fmt "\n", ##__VA_ARGS__)
#define LOGE(fmt, ...) printf("error: "   fmt "\n", ##__VA_ARGS__)
#endif


namespace android {


bool load_asset(char const* name, std::vector<uint8_t>& buf);
gfx::Texture2D* load_texture(char const*     name,
                             gfx::FilterMode filter = gfx::FilterMode::Nearest,
                             gfx::WrapMode   wrap   = gfx::WrapMode::Clamp);
std::string get_storage_dir();
void show_keyboard();
void hide_keyboard();


} // namespace
