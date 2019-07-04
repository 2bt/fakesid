cmake_minimum_required(VERSION 3.4.1)

project(exe)

#add_compile_options(-std=c++17 -Wall -O2)
add_compile_options(-std=c++17 -Wall -Og -g)

file(GLOB SRC
	"src/*.hpp"
	"src/*.cpp"
	)

add_executable(${PROJECT_NAME} ${SRC})

find_package(PkgConfig REQUIRED)
pkg_search_module(GLEW REQUIRED glew)

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
	${GLEW_INCLUDE_DIRS}
	)

target_link_libraries(${PROJECT_NAME}
	${GLEW_LIBRARIES}
    SDL2
	)
