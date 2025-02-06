#include "abbrev_table.hpp"
#include "cursor.hpp"
#include <cstdint>
#include <stdexcept>
#include <utility>

Abbrev_Table::Abbrev_Table (void *table_address) : data ((uint8_t *)table_address), cursor (table_address)
{
        uint64_t abbrev_code;

        while ((abbrev_code = this->cursor.uleb128 ()) != 0) {
                struct AbbrevTableEntry *entry = &this->table[abbrev_code];

                enum DW_CHILDREN child_indicator = (enum DW_CHILDREN)this->cursor.fixed<uint8_t> ();

                switch (child_indicator) {
                case DW_CHILDREN::yes: entry->has_children = true;
                case DW_CHILDREN::no: entry->has_children = false;
                default: throw std::logic_error ("Invalid DW_CHILDREN indicator!");
                }

                enum DW_AT attribute;
                enum DW_FORM form;

                while (attribute != DW_AT::nil && form != DW_FORM::nil) {
                        attribute = static_cast<enum DW_AT> (this->cursor.uleb128 ());
                        form = static_cast<enum DW_FORM> (this->cursor.uleb128 ());

                        entry->ordered_attributes.push_back (std::make_pair (attribute, form));

                        if (form == DW_FORM::implicit_const) {
                                int64_t attribute_value = this->cursor.sleb128 ();

                                entry->attribute_values[attribute] = attribute_value;
                        }
                }
        }
}

struct Abbrev_Table::AbbrevTableEntry &Abbrev_Table::find_entry (uint64_t abbrev_code)
{
        if (!this->table.count (abbrev_code))
                throw std::logic_error ("Abbreviation Table code not found!");

        return this->table[abbrev_code];
}
