cmake_minimum_required(VERSION 3.4.1)

project(fakesid)

add_compile_options(-std=c++17 -Wall -O3)

file(GLOB SRC
    src/resid-0.16/envelope.cc
    src/resid-0.16/extfilt.cc
    src/resid-0.16/filter.cc
    src/resid-0.16/pot.cc
    src/resid-0.16/sid.cc
    src/resid-0.16/version.cc
    src/resid-0.16/voice.cc
    src/resid-0.16/wave6581_PS_.cc
    src/resid-0.16/wave6581_PST.cc
    src/resid-0.16/wave6581_P_T.cc
    src/resid-0.16/wave6581__ST.cc
    src/resid-0.16/wave8580_PS_.cc
    src/resid-0.16/wave8580_PST.cc
    src/resid-0.16/wave8580_P_T.cc
    src/resid-0.16/wave8580__ST.cc
    src/resid-0.16/wave.cc

    src/app.cpp
    src/app.hpp
    src/edit.cpp
    src/edit.hpp
    src/foo.cpp
    src/foo.hpp
    src/gfx.cpp
    src/gfx.hpp
    src/gui.cpp
    src/gui.hpp
    src/instrument_effect_view.cpp
    src/instrument_effect_view.hpp
    src/jam_view.cpp
    src/jam_view.hpp
    src/main.cpp
    src/player.cpp
    src/player.hpp
    src/project_view.cpp
    src/project_view.hpp
    src/render.cpp
    src/render.hpp
    src/settings.cpp
    src/settings.hpp
    src/sid_engine.cpp
    src/sid_engine.hpp
    src/song.cpp
    src/song.hpp
    src/song_v0.hpp
    src/song_view.cpp
    src/song_view.hpp
    src/stb_image.h
    src/track_view.cpp
    src/track_view.hpp
    )

add_executable(${PROJECT_NAME} ${SRC})

find_package(PkgConfig REQUIRED)
pkg_search_module(GLEW REQUIRED glew)
pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SNDFILE REQUIRED sndfile)

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
    ${GLEW_INCLUDE_DIRS}
    )

target_link_libraries(${PROJECT_NAME} #PRIVATE
    ${GLEW_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SNDFILE_LIBRARIES}
    pthread
    )
