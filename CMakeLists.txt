cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
  imgui
  VERSION 0.1.0
  DESCRIPTION "Dear ImGui running in the browser"
  LANGUAGES CXX
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/imgui/)
