#include "RegisterMap.hpp"

#include <algorithm>

#include "BitUtils.hpp"

RegisterMap::RegisterMap()
{
    reset();
}

bool RegisterMap::isValidAddress(std::uint8_t address) const
{
    return address < Registers::REGISTER_COUNT;
}

bool RegisterMap::write(std::uint8_t address, std::uint32_t value)
{
    if (!isValidAddress(address))
    {
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    registers_[address] = value;
    return true;
}

bool RegisterMap::read(std::uint8_t address, std::uint32_t& value) const
{
    if (!isValidAddress(address))
    {
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    value = registers_[address];
    return true;
}

bool RegisterMap::setBit(std::uint8_t address, std::uint8_t bit)
{
    if (!isValidAddress(address))
    {
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    BitUtils::setBit(registers_[address], bit);
    return true;
}

bool RegisterMap::clearBit(std::uint8_t address, std::uint8_t bit)
{
    if (!isValidAddress(address))
    {
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    BitUtils::clearBit(registers_[address], bit);
    return true;
}

bool RegisterMap::isBitSet(std::uint8_t address, std::uint8_t bit, bool& result) const
{
    if (!isValidAddress(address))
    {
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    result = BitUtils::isBitSet(registers_[address], bit);
    return true;
}

DeviceSnapshot RegisterMap::snapshot() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    DeviceSnapshot snap{};
    snap.control = registers_[Registers::CONTROL];
    snap.status = registers_[Registers::STATUS];
    snap.fault = registers_[Registers::FAULT];
    snap.voltage_mv = registers_[Registers::VOLTAGE_MV];
    snap.temperature_cx10 = registers_[Registers::TEMPERATURE_CX10];
    snap.ov_threshold_mv = registers_[Registers::OV_THRESHOLD_MV];
    snap.ot_threshold_cx10 = registers_[Registers::OT_THRESHOLD_CX10];
    snap.device_id = registers_[Registers::DEVICE_ID];

    return snap;
}

void RegisterMap::reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::fill(registers_.begin(), registers_.end(), 0u);

    registers_[Registers::DEVICE_ID] = 0xB0011234u;
}
