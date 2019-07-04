cmake_minimum_required(VERSION 3.4.1)

project(Lib)

add_compile_options(-std=c++17 -O2 -Wall)

add_library(${PROJECT_NAME} SHARED
    cpp/main.cpp
    cpp/app.cpp
    cpp/gfx.cpp
    cpp/foo.cpp
    cpp/gui.cpp
    )

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
    #/home/twobit/Programming/android/lib/
    ../../../../../../lib
    )

# add lib dependencies
target_link_libraries(${PROJECT_NAME}
    android
    log
    EGL
    GLESv2
    )

