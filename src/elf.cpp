#include "elf.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <elf.h>
#include <stdio.h>
ElfReader::ElfReader (const char *elf_file_path) : m_elf_file_path (elf_file_path)
{
        FILE *fp = fopen (this->m_elf_file_path, "rb");

        if (!fp) {
                perror ("fopen");
                exit (EXIT_FAILURE);
        }

        fseek (fp, 0, SEEK_END);

        long file_size = ftell (fp);

        rewind (fp);

        this->m_elf_data = (uint8_t *)malloc (file_size);

        if (!this->m_elf_data) {
                perror ("malloc");
                exit (EXIT_FAILURE);
        }

        if (fread (this->m_elf_data, file_size, 1, fp) != 1) {
                perror ("fread");
                exit (EXIT_FAILURE);
        }

        fclose (fp);

        Elf64_Shdr *sh_string_table = this->section_header (this->header ()->e_shstrndx);

        this->m_string_table = (char *)this->header () + sh_string_table->sh_offset;

}

Elf64_Ehdr *ElfReader::header ()
{
        return (Elf64_Ehdr *)this->m_elf_data;
}

Elf64_Shdr *ElfReader::first_section_header ()
{
        unsigned long offset = this->header ()->e_shoff;

        return (Elf64_Shdr *)(this->m_elf_data + offset);
}

Elf64_Shdr *ElfReader::section_header (int index)
{
        return &this->first_section_header ()[index];
}

void *ElfReader::section_contents (int header_index)
{
        Elf64_Shdr *header = this->section_header(header_index);

        return (char *)this->header () + header->sh_offset;
}

const char *ElfReader::section_name (int name_offset)
{
        return &this->m_string_table[name_offset];
}

size_t ElfReader::section_size (int index)
{
        Elf64_Shdr *header = this->section_header (index);

        return header->sh_size;
}

int ElfReader::find_section_index (const char *name)
{
        for (int i = 0; i < this->header ()->e_shnum; i++) {
                Elf64_Shdr *section = this->section_header (i);
                const char *section_name = this->section_name (section->sh_name);

                if (strcmp (section_name, name) == 0)
                        return i;
        }

        return -1;
}

void ElfReader::debug ()
{
        Elf64_Shdr *sh_string_table = this->section_header (this->header ()->e_shstrndx);

        this->m_string_table = (char *)this->header () + sh_string_table->sh_offset;

        for (int i = 0; i < this->header ()->e_shnum; i++) {
                Elf64_Shdr *sh = this->section_header (i);

                const char *name = &this->m_string_table[sh->sh_name];

                printf ("%s\n", name);
        }
}
