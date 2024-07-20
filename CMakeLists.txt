cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
  sca
  VERSION 2.0.0
  DESCRIPTION "STALCRAFT Auction data visualized"
  LANGUAGES CXX
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/imgui/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/server/)
