#include "cursor.hpp"
#include <cstdint>
#include <stdexcept>

#define LOW7_MASK (0b01111111)

Cursor::Cursor (void *data, size_t size) : data ((uint8_t *)data), size (size)
{
        pos = (uint8_t *)data;
}

uint8_t Cursor::peek ()
{
        return *pos;
}

void Cursor::next ()
{
        if (pos - data > size - 1)
                throw std::logic_error ("Unexpected end of section.");

        pos++;
}

void Cursor::offset (size_t off)
{
        pos = data + off;
}

bool Cursor::is_end ()
{
        return pos >= (data + size);
}

void *Cursor::address ()
{
        return pos;
}

int64_t Cursor::sleb128 ()
{
        uint64_t result = 0;
        int shift = 0;
        size = 64;

        uint64_t byte;

        while (true) {
                byte = this->peek ();
                result |= (byte & LOW7_MASK) << shift;
                shift += 7;
                this->next ();

                if (byte & ~LOW7_MASK)
                        break;
        }

        if (shift < size && (byte & ~LOW7_MASK))
                result |= -(1 << shift);

        return result;
}

uint64_t Cursor::uleb128 ()
{
        uint64_t result = 0;
        int shift = 0;

        while (true) {
                uint8_t byte = this->peek ();

                result |= (byte & LOW7_MASK) << shift;

                this->next ();

                if ((byte & ~LOW7_MASK) == 0)
                        break;

                shift += 7;
        }

        return result;
}
