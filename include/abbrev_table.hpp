#pragma once

#include "cursor.hpp"
#include "data.hpp"
#include <cstdint>
#include <map>
#include <vector>
class Abbrev_Table {


    public:
    struct AbbrevTableEntry {
        bool has_children;
        std::vector<std::pair<enum DW_AT, enum DW_FORM>> ordered_attributes;
        std::map<enum DW_AT, int64_t> attribute_values;
    
    };

    uint8_t *data;
    std::map<uint64_t, struct AbbrevTableEntry> table;
    Cursor cursor;
    Abbrev_Table(void *table_address);

    struct AbbrevTableEntry& find_entry(uint64_t abbrev_code);


};