cmake_minimum_required(VERSION 3.30)

project(dbg C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


link_directories(${PROJECT_SOURCE_DIR}/lib/libelfin/dwarf ${PROJECT_SOURCE_DIR}/lib/libelfin/elf)

add_executable(dbg "src/main.cpp")

add_library(breakpoint SHARED "src/breakpoint.cpp")

include_directories(include)
include_directories(lib/libelfin/dwarf)
include_directories(lib/libelfin/elf)
target_link_libraries(dbg breakpoint dwarf++ elf++)



