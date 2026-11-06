cmake_minimum_required(VERSION 3.14)
project(my_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS include/*.hpp)

add_executable(my_app ${SOURCES} ${HEADERS})
target_include_directories(my_app PRIVATE include)

if(MSVC)
  target_compile_options(my_app PRIVATE /utf-8)
  target_compile_definitions(my_app PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()
