project(robotradar)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/sdl2-cmake-scripts")

#bin/sdl2-config --cflags
#INCLUDE_DIRECTORIES("/usr/local/include/SDL2")
#add_definitions(-D_THREAD_SAFE)

#bin/sdl2-config --libs
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})

