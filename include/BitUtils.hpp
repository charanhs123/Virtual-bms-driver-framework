#ifndef BITUTILS_HPP
#define BITUTILS_HPP

#include <cstdint>

namespace BitUtils
{
inline void setBit(std::uint32_t& value, std::uint8_t bit)
{
    value |= (1u << bit);
}

inline void clearBit(std::uint32_t& value, std::uint8_t bit)
{
    value &= ~(1u << bit);
}

inline bool isBitSet(std::uint32_t value, std::uint8_t bit)
{
    return (value & (1u << bit)) != 0u;
}

inline void writeBit(std::uint32_t& value, std::uint8_t bit, bool set)
{
    if (set)
    {
        setBit(value, bit);
    }
    else
    {
        clearBit(value, bit);
    }
}

inline std::uint16_t clampToU16(std::uint32_t value)
{
    return (value > 0xFFFFu) ? 0xFFFFu : static_cast<std::uint16_t>(value);
}
}

#endif