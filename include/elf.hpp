#pragma once
#include <cstddef>
#include <cstdint>
#include <elf.h>

class ElfReader {
    public:
        ElfReader (const char *elf_file_path);

        Elf64_Ehdr *header ();
        Elf64_Shdr *first_section_header ();
        Elf64_Shdr *section_header(int index);
        void *section_contents(int header_index);
        int find_section_index(const char *name);
        const char *section_name(int index);
        size_t section_size(int index);
        void *debug_info();
        void *offset(size_t offset);
        void debug ();

    private:
        const char *m_elf_file_path;
        uint8_t *m_elf_data;
        const char *m_string_table;
};
