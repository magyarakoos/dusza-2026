cmake_minimum_required(VERSION 3.14)
project(my_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
  GIT_TAG        v7.0.3
)
FetchContent_MakeAvailable(ftxui)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS include/*.hpp)

add_executable(my_app ${SOURCES} ${HEADERS})
target_include_directories(my_app PRIVATE include)
target_link_libraries(my_app PRIVATE ftxui::screen ftxui::dom ftxui::component)

if(MSVC)
  target_compile_options(my_app PRIVATE /utf-8)
  target_compile_definitions(my_app PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()
