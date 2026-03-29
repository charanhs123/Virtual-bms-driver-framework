#include "DriverTypes.hpp"

#include "BitUtils.hpp"

std::string faultRegisterToString(std::uint32_t fault_reg)
{
    if (fault_reg == 0u)
    {
        return "NONE";
    }

    std::string result;

    if (BitUtils::isBitSet(fault_reg, FaultBits::OVERVOLTAGE))
    {
        result += "OVERVOLTAGE ";
    }

    if (BitUtils::isBitSet(fault_reg, FaultBits::OVERTEMPERATURE))
    {
        result += "OVERTEMPERATURE ";
    }

    if (!result.empty() && result.back() == ' ')
    {
        result.pop_back();
    }

    return result;
}

std::string statusRegisterToString(std::uint32_t status_reg)
{
    std::string result;

    if (BitUtils::isBitSet(status_reg, StatusBits::DEVICE_READY))
    {
        result += "READY ";
    }

    if (BitUtils::isBitSet(status_reg, StatusBits::MEASUREMENT_VALID))
    {
        result += "MEAS_VALID ";
    }

    if (BitUtils::isBitSet(status_reg, StatusBits::FAULT_ACTIVE))
    {
        result += "FAULT_ACTIVE ";
    }

    if (result.empty())
    {
        return "NONE";
    }

    if (result.back() == ' ')
    {
        result.pop_back();
    }

    return result;
}