cmake_minimum_required(VERSION 3.17)

project("Inventory Monte Carlo")

add_library(imgui
  extern/imgui/imgui.cpp
  extern/imgui/imgui_demo.cpp
  extern/imgui/imgui_draw.cpp
  extern/imgui/imgui_widgets.cpp
)

target_include_directories(imgui
  PUBLIC extern/imgui
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(extern/glfw)

add_library(imguiGLFW
  extern/imgui/examples/imgui_impl_glfw.cpp
)

target_include_directories(imguiGLFW
  PUBLIC extern/imgui/examples
)

target_link_libraries(imguiGLFW
  PUBLIC imgui
  PUBLIC glfw
)

include(FindOpenGL)
include(FindGLEW)

add_library(imguiOpenGL
  extern/imgui/examples/imgui_impl_opengl3.cpp
)

target_include_directories(imguiOpenGL
  PUBLIC extern/imgui/examples
)

target_link_libraries(imguiOpenGL
  PUBLIC imgui
  PUBLIC OpenGL::GL
  PUBLIC GLEW::glew
)

add_executable(inventoryMonteCarlo
  src/main.cpp
)

target_link_libraries(inventoryMonteCarlo
  PRIVATE imguiGLFW
  PRIVATE imguiOpenGL
)

if(MSVC)
target_compile_options(inventoryMonteCarlo
  PRIVATE /W4
  PRIVATE /WX
)
else()
target_compile_options(inventoryMonteCarlo
  PRIVATE -Wall
  PRIVATE -Wextra
  PRIVATE -pedantic
  PRIVATE -Werror
)
endif()
