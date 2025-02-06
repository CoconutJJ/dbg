#pragma once
#include "elf.hpp"
class Dwarf {

    public:
    ElfReader *reader;
    void *debug_info_contents;
    size_t debug_info_size;

    void *debug_abbrev_contents;
    size_t debug_abbrev_size;

    Dwarf(ElfReader *reader);

    void debug_info();
    void debug_abbrev();
};