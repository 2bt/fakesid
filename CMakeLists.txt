cmake_minimum_required(VERSION 3.4.1)

project(fakesid)

#add_compile_options(-std=c++17 -Wall -O2)
add_compile_options(-std=c++17 -Wall -Og -g)

file(GLOB SRC
    "src/*.hpp"
    "src/*.cpp"
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
