cmake_minimum_required(VERSION 3.25)
project(implot)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

file(GLOB_RECURSE source CONFIGURE_DEPENDS "*.cpp")

add_library(implot ${source})
target_include_directories(implot PUBLIC include/)
