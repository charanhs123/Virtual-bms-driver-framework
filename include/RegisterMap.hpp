#ifndef REGISTERMAP_HPP
#define REGISTERMAP_HPP

#include <array>
#include <cstdint>
#include <mutex>

#include "DriverTypes.hpp"

class RegisterMap
{
public:
    RegisterMap();

    bool write(std::uint8_t address, std::uint32_t value);
    bool read(std::uint8_t address, std::uint32_t& value) const;

    bool setBit(std::uint8_t address, std::uint8_t bit);
    bool clearBit(std::uint8_t address, std::uint8_t bit);
    bool isBitSet(std::uint8_t address, std::uint8_t bit, bool& result) const;

    DeviceSnapshot snapshot() const;
    void reset();

private:
    bool isValidAddress(std::uint8_t address) const;

    mutable std::mutex mutex_;
    std::array<std::uint32_t, Registers::REGISTER_COUNT> registers_;
};

#endif