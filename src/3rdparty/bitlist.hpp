#pragma once

#include <cstdint>

class bitlist
{
    constexpr static int BITS = 8;

protected:
    using bigsize = std::size_t;

    bigsize _size;
    int *_buffer;
    void _realloc();

public:
    bitlist();
    bitlist(bigsize size);

    ~bitlist();

    bigsize size() const;
    bool empty() const;

    void set(bigsize bitPos, int bitValue);
    int get(bigsize bitPos);
};

bitlist::bitlist() : _size(0), _buffer(nullptr)
{
}

bitlist::bitlist(bigsize size) : _size(size), _buffer(nullptr)
{
    _realloc();
}

bitlist::~bitlist()
{
    if(_buffer)
        delete[] _buffer;
    _size = 0;
}

void bitlist::_realloc()
{
    if(_buffer)
        delete[] _buffer;

    _buffer = new int[_size] {0};
}

bitlist::bigsize bitlist::size() const
{
    return _size;
}

bool bitlist::empty() const
{
    return _size == 0;
}

void bitlist::set(bigsize bitPos, int bitValue)
{
    bigsize quot = bitPos / BITS;
    bigsize rem = bitPos % BITS;

    std::uint8_t *pointer = (reinterpret_cast<std::uint8_t *>(_buffer)) + quot;
    quot = (1 << rem);
    // Reset bit
    (*pointer) ^= (*pointer) & (quot);
    // Write bit
    (*pointer) |= quot * (bitValue == 1);
}

int bitlist::get(bigsize bitPos)
{
    bigsize quot = bitPos / BITS;
    bigsize rem = bitPos % BITS;

    std::uint8_t *pointer = (reinterpret_cast<std::uint8_t *>(_buffer)) + quot;
    quot = (1 << rem);
    return static_cast<int>((*pointer) & quot) & 0x1;
}
