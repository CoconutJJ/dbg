#include "dwarf.hpp"
#include "elf.hpp"

int main(int argc, char **argv) {

    ElfReader erdr(argv[1]);

    Dwarf dwarf(&erdr);

    dwarf.debug_info();


}