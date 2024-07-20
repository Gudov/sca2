cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE "Release")

set(CMAKE_CXX_COMPILER em++)

#=====================[Project Description]====================== #

project(
  imgui
  VERSION 0.1.0
  DESCRIPTION "Dear ImGui running in the browser"
  LANGUAGES CXX
)

#======================[Target Description]====================== #

add_executable(imgui)

file(GLOB_RECURSE SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp

  imgui/backends/imgui_impl_glfw.cpp
  imgui/backends/imgui_impl_opengl3.cpp
  imgui/imgui.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_demo.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui_tables.cpp
)

target_compile_features(
  imgui
  PRIVATE
    cxx_std_23
)

target_include_directories(
  imgui
  PRIVATE
    src/
    imgui
    imgui/backends
)

target_sources(
  imgui

  PRIVATE
    ${SOURCES}
)

target_link_libraries(
  imgui

  PRIVATE
    GL
    websocket.js
)

target_link_options(
  imgui

  PRIVATE
    "SHELL:-s USE_WEBGL2=1"
    "SHELL:-s USE_GLFW=3"
    "SHELL:-s FULL_ES3=1"
    "SHELL:--preload-file ../data"
)

set(RELEASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/www/)

set_target_properties(
  imgui

  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${RELEASE_DIR}
)
