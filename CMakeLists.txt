cmake_minimum_required(VERSION 3.17)

project("Inventory Monte Carlo")

add_library(imgui
  imgui/imgui.cpp
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_widgets.cpp
)

target_include_directories(imgui
  PUBLIC imgui
)

find_package(glfw3 REQUIRED)

add_library(imguiGLFW
  imgui/examples/imgui_impl_glfw.cpp
)

target_include_directories(imguiGLFW
  PUBLIC imgui/examples
)

target_link_libraries(imguiGLFW
  PUBLIC imgui
  PUBLIC glfw
)

include(FindOpenGL)
include(FindGLEW)

add_library(imguiOpenGL
  imgui/examples/imgui_impl_opengl3.cpp
)

target_include_directories(imguiOpenGL
  PUBLIC imgui/examples
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

target_compile_options(inventoryMonteCarlo
  PRIVATE -Wall
  PRIVATE -Wextra
  PRIVATE -pedantic
  PRIVATE -Werror
)
