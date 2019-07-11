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

# add lib dependencies
target_link_libraries(${PROJECT_NAME}
    android
    log
    EGL
    GLESv2
    )
