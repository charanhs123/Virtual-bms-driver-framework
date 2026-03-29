#ifndef DRIVERTYPES_HPP
#define DRIVERTYPES_HPP

#include <cstdint>
#include <string>

namespace Registers
{
constexpr std::uint8_t CONTROL = 0x00;
constexpr std::uint8_t STATUS = 0x01;
constexpr std::uint8_t FAULT = 0x02;
constexpr std::uint8_t VOLTAGE_MV = 0x03;
constexpr std::uint8_t TEMPERATURE_CX10 = 0x04;
constexpr std::uint8_t OV_THRESHOLD_MV = 0x05;
constexpr std::uint8_t OT_THRESHOLD_CX10 = 0x06;
constexpr std::uint8_t DEVICE_ID = 0x07;
constexpr std::uint8_t REGISTER_COUNT = 0x08;
}

namespace ControlBits
{
constexpr std::uint8_t ENABLE_MEASUREMENT = 0;
constexpr std::uint8_t ENABLE_FAULT_DETECTION = 1;
constexpr std::uint8_t CLEAR_FAULTS = 2;
}

namespace StatusBits
{
constexpr std::uint8_t DEVICE_READY = 0;
constexpr std::uint8_t MEASUREMENT_VALID = 1;
constexpr std::uint8_t FAULT_ACTIVE = 2;
}

namespace FaultBits
{
constexpr std::uint8_t OVERVOLTAGE = 0;
constexpr std::uint8_t OVERTEMPERATURE = 1;
}

struct MeasurementData
{
    double voltage_mv;
    double temperature_c;
};

struct ThresholdConfig
{
    std::uint16_t overvoltage_mv;
    std::uint16_t overtemperature_cx10;
};

struct DeviceSnapshot
{
    std::uint32_t control;
    std::uint32_t status;
    std::uint32_t fault;
    std::uint32_t voltage_mv;
    std::uint32_t temperature_cx10;
    std::uint32_t ov_threshold_mv;
    std::uint32_t ot_threshold_cx10;
    std::uint32_t device_id;
};

std::string faultRegisterToString(std::uint32_t fault_reg);
std::string statusRegisterToString(std::uint32_t status_reg);

#endif