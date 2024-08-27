cmake_minimum_required(VERSION 3.27)

#set(CMAKE_SYSTEM_NAME Windows)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(CMAKE_C_COMPILER   x86_64-w64-mingw32-gcc)
	set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
	set(CMAKE_RC_COMPILER  x86_64-w64-mingw32-windres)
	set(CMAKE_RANLIB       x86_64-w64-mingw32-ranlib)

	set(CMAKE_FIND_ROOT_PATH "/usr/x86_64-w64-mingw32")
	set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
	set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
	set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
else()
	set(CMAKE_C_COMPILER   gcc)
	set(CMAKE_CXX_COMPILER g++)
endif()

set(CMAKE_CXX_FLAGS  "-Wall -Wextra -g")
set(CMAKE_C_FLAGS    "-Wall -Wextra -g")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

project(gibmeaname)

# OpenGL
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
#include_directories(src ${OPENGL_INCLUDE_DIRS})

add_subdirectory(libs/stbi)
add_subdirectory(libs/glad)
add_subdirectory(libs/imgui)
add_subdirectory(libs/implot)

# set "cppsource" variables to all .c and .cpp files in "src"
file(GLOB_RECURSE cppsource CONFIGURE_DEPENDS "main.cpp")
file(GLOB_RECURSE csource   CONFIGURE_DEPENDS "")

add_executable(sim ${csource} ${cppsource})
target_link_libraries(sim ${OPENGL_LIBRARIES} glad stb_image imgui implot ginac cln)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	target_link_libraries(sim -lm -lgdi32 -luser32 -lkernel32)
	target_link_libraries(sim glfw3)
else()
	target_link_libraries(sim -lglfw -lGL -lrt -lm -lX11 -lpthread -lXrandr -lXi -ldl -lxcb -lXau -lXdmcp)
endif()

add_custom_command(
	TARGET sim
	POST_BUILD
	COMMAND ${CMAKE_COMMAND}
	ARGS -E copy $<TARGET_FILE:sim> ${PROJECT_SOURCE_DIR}
)

