cmake_minimum_required(VERSION 3.4.1)

set(OBOE_DIR "../../../../../oboe")
set(SNDFILE_DIR "../../../../../../lib/libsndfile")

project(main)

add_subdirectory(${SNDFILE_DIR} ./sndfile-bin)
add_subdirectory(${OBOE_DIR} ./oboe-bin)

add_compile_options(-std=c++17 -O3 -Wall)

file(GLOB SRC
    "cpp/*.hpp"
    "cpp/*.cpp"
    )

add_library(${PROJECT_NAME} SHARED ${SRC})

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
    "../../../../../../lib" # glm
    "${OBOE_DIR}/include"
    "${OBOE_DIR}/src"
    "${SNDFILE_DIR}"
    )

target_link_libraries(${PROJECT_NAME}
    android
    log
    EGL
    GLESv2
    sndfile
    oboe
    atomic
    )
