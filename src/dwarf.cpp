#include "dwarf.hpp"
#include "abbrev_table.hpp"
#include "cursor.hpp"
#include "data.hpp"
#include <cstdint>
#include <stdexcept>

#define BYTE4_MAX_INT 0xffffffff

Dwarf::Dwarf (ElfReader *reader) : reader (reader)
{
        int debug_info_index = reader->find_section_index (".debug_info");

        this->debug_info_contents = reader->section_contents (debug_info_index);
        this->debug_info_size = reader->section_size (debug_info_index);

        int debug_abbrev_index = reader->find_section_index (".debug_abbrev");
        this->debug_abbrev_contents = reader->section_contents (debug_abbrev_index);
        this->debug_abbrev_size = reader->section_size (debug_abbrev_index);
}

void Dwarf::debug_info ()
{
        Cursor debug_info_cursor (this->debug_info_contents, this->debug_info_size);

        uint32_t magic = debug_info_cursor.fixed<uint32_t> ();

        if (magic != BYTE4_MAX_INT)
                throw std::logic_error ("32 bit DWARF not supported!");

        uint64_t initial_length = debug_info_cursor.fixed<uint64_t> ();

        uint16_t version = debug_info_cursor.fixed<uint16_t> ();

        if (version != 5)
                throw std::logic_error ("Only DWARF v5 is supported!");

        enum DW_UT unit_type = (enum DW_UT)debug_info_cursor.fixed<uint8_t> ();

        uint8_t address_size = debug_info_cursor.fixed<uint8_t> ();

        uint64_t debug_abbrev_offset = debug_info_cursor.fixed<uint64_t> ();

        // DIE Entry Begin:
        uint64_t abbrev_code = debug_info_cursor.uleb128 ();

        printf ("Abbrev Code: %lu\n", abbrev_code);

        Abbrev_Table abbrev_table(this->debug_abbrev_contents);

        struct Abbrev_Table::AbbrevTableEntry entry = abbrev_table.find_entry(abbrev_code);

        for (auto attr_pair : entry.ordered_attributes) {

                enum DW_AT attr = attr_pair.first;
                enum DW_FORM form = attr_pair.second;

                
                



        }


}
