cmake_minimum_required(VERSION 3.30)

project(dbg C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_CXX_FLAGS  "-ggdb")

link_directories(${PROJECT_SOURCE_DIR}/lib/libelfin/dwarf ${PROJECT_SOURCE_DIR}/lib/libelfin/elf)

# add_executable(dbg "src/main.cpp")
add_executable(elf_debug "src/elf_debug.cpp")
# add_library(debugger "src/debugger.cpp")
# add_library(breakpoint "src/breakpoint.cpp")
add_library(elf "src/elf.cpp")
add_library(dwarf "src/dwarf.cpp" "src/cursor.cpp")

include_directories(include)
# include_directories(lib/libelfin/dwarf)
# include_directories(lib/libelfin/elf)
# target_link_libraries(dbg breakpoint debugger dwarf++ elf++)
target_link_libraries(elf_debug elf dwarf)


