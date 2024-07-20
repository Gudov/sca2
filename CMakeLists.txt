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

add_custom_target(T1 DEPENDS t1.txt)
add_custom_target(default ALL DEPENDS t1.txt)

add_custom_command(
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/version sca ${CMAKE_CURRENT_SOURCE_DIR}/shared/version.hpp
  OUTPUT t1.txt
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/imgui/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/server/)
