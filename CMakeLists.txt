cmake_minimum_required(VERSION 3.4.1)

project(fakesid)

add_compile_options(-std=c++17 -Wall -O3)

add_executable(${PROJECT_NAME}
    src/app.cpp
    src/edit.cpp
    src/foo.cpp
    src/gfx.cpp
    src/gui.cpp
    src/instrument_effect_view.cpp
    src/jam_view.cpp
    src/main.cpp
    src/player.cpp
    src/project_view.cpp
    src/render.cpp
    src/settings.cpp
    src/sid_engine.cpp
    src/song.cpp
    src/song_view.cpp
    src/stb_image.h
    src/track_view.cpp
)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLEW REQUIRED glew)
pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SNDFILE REQUIRED sndfile)


target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
    ${GLEW_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SNDFILE_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} #PRIVATE
    ${GLEW_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SNDFILE_LIBRARIES}
    pthread
)
