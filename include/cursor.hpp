#pragma once

#include <cstddef>
#include <cstdint>
class Cursor {
    public:
        uint8_t *data;
        size_t size;
        uint8_t *pos;

        Cursor (void *data, size_t size);
        Cursor (void *data);
        uint8_t peek ();
        void next ();
        bool is_end();

        uint64_t uleb128 ();
        int64_t sleb128 ();
        
        void *address();

        template <typename T>
        T fixed ()
        {
                uint64_t result;
                uint8_t *result_ptr = (uint8_t*)&result;
                for (int i = 0; i < sizeof (T); i++) {
                        *result_ptr = this->peek ();
                        this->next ();
                        result_ptr++;
                }

                return T (result);
        }

        void offset (size_t off);
};