set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_COMPILER em++)

add_executable(imgui)
add_dependencies(imgui version)

# SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fwasm-exceptions -g")

file(GLOB_RECURSE SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp

  ${CMAKE_CURRENT_SOURCE_DIR}/../shared/*.cpp

  imgui/backends/imgui_impl_glfw.cpp
  imgui/backends/imgui_impl_opengl3.cpp
  imgui/imgui.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_demo.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui_tables.cpp
  imgui/misc/cpp/imgui_stdlib.cpp
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
    imgui/
    imgui/misc/cpp/
    imgui/backends/
    ../shared/
    $ENV{EMSDK}/upstream/emscripten/cache/sysroot/include/fakesdl
    $ENV{EMSDK}/upstream/emscripten/cache/sysroot/include/compat
    $ENV{EMSDK}/upstream/emscripten/cache/sysroot/include/c++/v1
    $ENV{EMSDK}/upstream/lib/clang/19/include
    $ENV{EMSDK}/upstream/emscripten/cache/sysroot/include
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
    # "SHELL:--preload-file ../data"
    "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/data@/"
    -O2
)

set(RELEASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/www/)

set_target_properties(
  imgui
  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${RELEASE_DIR}
    OUTPUT_NAME "imgui.js"
)
