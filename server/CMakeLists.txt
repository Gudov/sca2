option(USE_STANDALONE_ASIO "set ON to use standalone Asio instead of Boost.Asio" ON)
option(BUILD_SHARED_LIBS "Build libraries as shared libraries" OFF)

add_executable(server)
add_dependencies(server version)

file(GLOB_RECURSE SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
)

target_include_directories(
  server
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
   ./src
  ../shared
)

target_sources(
  server
  PRIVATE
  ${SOURCES}
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cpr)
target_link_libraries(
  server
  PRIVATE
  cpr::cpr
)

find_package(Threads REQUIRED)
target_link_libraries(server PRIVATE Threads::Threads)

find_package(OpenSSL REQUIRED)
target_link_libraries(server PRIVATE OpenSSL::SSL OpenSSL::Crypto)

add_compile_options(-Wall -Wextra -Wsign-conversion)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wthread-safety)
endif()

# TODO 2020 when Debian Jessie LTS ends:
# Remove Boost system, thread, regex components; use Boost::<component> aliases; remove Boost target_include_directories
if(USE_STANDALONE_ASIO)
  target_compile_definitions(server PRIVATE ASIO_STANDALONE)
  # There is no canonical way to use Asio from CMake.
  # In particular, Asio does not support CMake natively.
  # However, Conan and Vcpkg do provide CMake support on their own.
  # Prefer the CMake target and fall back to finding asio.hpp.
  if(NOT TARGET asio::asio)
    find_package(asio)
  endif()
  if(TARGET asio::asio)
    target_link_libraries(server PRIVATE asio::asio)
  else()
    find_path(ASIO_PATH asio.hpp)
    if(NOT ASIO_PATH)
      message(FATAL_ERROR "Standalone Asio not found")
    endif()
    target_include_directories(server SYSTEM PRIVATE ${ASIO_PATH})
  endif()
else()
  find_package(Boost 1.54.0 COMPONENTS system REQUIRED)
  target_link_libraries(server PRIVATE Boost::boost Boost::system)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
    target_compile_definitions(server PRIVATE USE_BOOST_REGEX)
    find_package(Boost 1.54.0 COMPONENTS regex REQUIRED)
    target_link_libraries(server PRIVATE Boost::regex)
  endif()
endif()

set_target_properties(server PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
set(server CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
