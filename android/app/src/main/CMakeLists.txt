cmake_minimum_required(VERSION 3.4.1)

project(main)

add_compile_options(-std=c++17 -O2 -Wall)

file(GLOB SRC
	"cpp/*.hpp"
	"cpp/*.cpp"
    )

add_library(${PROJECT_NAME} SHARED ${SRC})

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
    "../../../../../../lib"
    )

target_link_libraries(${PROJECT_NAME}
    android
    log
    EGL
    GLESv2
    )


set(OBOE_DIR "../../../../../oboe")

add_subdirectory(${OBOE_DIR} ./oboe-bin)

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
    "${OBOE_DIR}/include"
    "${OBOE_DIR}/src"
    )

target_link_libraries(${PROJECT_NAME}
    oboe
    atomic
    )
