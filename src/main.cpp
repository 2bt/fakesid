#include "app.hpp"
#include "foo.hpp"

#ifdef ANDROID

#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

AAssetManager* g_asset_manager;
JNIEnv*        g_env;

extern "C" {
    JNIEXPORT void JNICALL Java_com_twobit_fakesid_Lib_init(JNIEnv * env, jobject obj, jobject am) {
        g_env = env;
        g_asset_manager = AAssetManager_fromJava(env, am);
        app::init();
    }
    JNIEXPORT void JNICALL Java_com_twobit_fakesid_Lib_free(JNIEnv * env, jobject obj) {
        app::free();
    }
    JNIEXPORT void JNICALL Java_com_twobit_fakesid_Lib_exit(JNIEnv * env, jobject obj) {
        app::exit();
    }
    JNIEXPORT void JNICALL Java_com_twobit_fakesid_Lib_resize(JNIEnv * env, jobject obj, jint width, jint height) {
        app::resize(width, height);
    }
    JNIEXPORT void JNICALL Java_com_twobit_fakesid_Lib_draw(JNIEnv * env, jobject obj) {
        app::draw();
    }
    JNIEXPORT void JNICALL Java_com_twobit_fakesid_Lib_touch(JNIEnv * env, jobject obj, jint x, jint y, jint action) {
        app::touch(x, y, action == 0 || action == 2);
    }
    JNIEXPORT void JNICALL Java_com_twobit_fakesid_Lib_key(JNIEnv * env, jobject obj, jint key, jint unicode) {
        app::key(key, unicode);
    }
}

#else

#include <SDL2/SDL.h>
#include <GL/glew.h>

namespace {

SDL_Window*   s_window;
bool          s_running       = true;
int           s_screen_width  = 600;
int           s_screen_height = 1000;
SDL_GLContext s_gl_context;

void free() {
    SDL_GL_DeleteContext(s_gl_context);
    SDL_DestroyWindow(s_window);
    SDL_Quit();
}

} // namespace

int main(int argc, char** argv) {

    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);

    s_window = SDL_CreateWindow(
            "app",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            s_screen_width, s_screen_height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    s_gl_context = SDL_GL_CreateContext(s_window);
    if (!s_gl_context) {
        fprintf(stderr, "error: SDL_GL_CreateContext() failed\n");
        free();
        return 1;
    }

    SDL_GL_SetSwapInterval(1);
    glewExperimental = true;
    glewInit();

    app::init();

    while (s_running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                s_running = false;
                break;

            case SDL_KEYDOWN:
                switch (e.key.keysym.scancode) {
                case SDL_SCANCODE_ESCAPE:
                    s_running = false;
                    break;
                case SDL_SCANCODE_RETURN:
                    app::key(KEYCODE_ENTER, 0);
                    break;
                case SDL_SCANCODE_BACKSPACE:
                    app::key(KEYCODE_DEL, 0);
                    break;
                default: break;
                }
                break;
            case SDL_TEXTINPUT:
                app::key(0, e.text.text[0]);
                break;

            case SDL_WINDOWEVENT:
                if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    s_screen_width  = e.window.data1;
                    s_screen_height = e.window.data2;
                    app::resize(s_screen_width, s_screen_height);
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (e.button.button != SDL_BUTTON_LEFT) break;
                app::touch(e.motion.x, e.motion.y, true);
                break;
            case SDL_MOUSEBUTTONUP:
                if (e.button.button != SDL_BUTTON_LEFT) break;
                app::touch(e.motion.x, e.motion.y, false);
                break;
            case SDL_MOUSEMOTION:
                if (!(e.motion.state & SDL_BUTTON_LMASK)) break;
                app::touch(e.motion.x, e.motion.y, true);
                break;

            default: break;
            }
        }

        app::draw();
        SDL_GL_SwapWindow(s_window);
    }

    app::exit();
    free();
    return 0;
}

#endif
