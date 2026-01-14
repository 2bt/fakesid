#pragma once
#include "gfx.hpp"

#ifdef ANDROID
#include <android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO,  "FOOBAR", __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "FOOBAR", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN,  "FOOBAR", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "FOOBAR", __VA_ARGS__))
#else
#include <cstdio>
#define LOGI(fmt, ...) printf(          fmt "\n", ##__VA_ARGS__)
#define LOGD(fmt, ...) printf("DEBUG: " fmt "\n", ##__VA_ARGS__)
#define LOGW(fmt, ...) printf("WARN:  " fmt "\n", ##__VA_ARGS__)
#define LOGE(fmt, ...) printf("ERROR: " fmt "\n", ##__VA_ARGS__)
#endif


namespace android {

bool start_audio();
void stop_audio();

bool load_asset(std::string const& name, std::vector<uint8_t>& buf);
gfx::Texture2D* load_texture(char const*     name,
                             gfx::FilterMode filter = gfx::FilterMode::Nearest,
                             gfx::WrapMode   wrap   = gfx::WrapMode::Clamp);
void show_keyboard();
void hide_keyboard();
void export_song(std::string const& path, std::string const& title);
void start_song_import();
void update_setting(int i);

} // namespace
