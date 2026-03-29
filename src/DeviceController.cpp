#include "DeviceController.hpp"

#include <cstdint>

#include "BitUtils.hpp"

namespace
{
void updateControlBit(VirtualBatteryMonitor& device, std::uint8_t bit, bool enable)
{
    std::uint32_t control = 0u;
    device.readRegister(Registers::CONTROL, control);
    BitUtils::writeBit(control, bit, enable);
    device.writeRegister(Registers::CONTROL, control);
}
}

DeviceController::DeviceController()
{
    initialize();
}

void DeviceController::initialize()
{
    device_.initialize();
}

void DeviceController::enableMeasurement(bool enable)
{
    updateControlBit(device_, ControlBits::ENABLE_MEASUREMENT, enable);
}

void DeviceController::enableFaultDetection(bool enable)
{
    updateControlBit(device_, ControlBits::ENABLE_FAULT_DETECTION, enable);
}

void DeviceController::clearFaults()
{
    std::uint32_t control = 0u;
    device_.readRegister(Registers::CONTROL, control);
    BitUtils::setBit(control, ControlBits::CLEAR_FAULTS);
    device_.writeRegister(Registers::CONTROL, control);
}

void DeviceController::configureThresholds(std::uint16_t overvoltage_mv,
                                           std::uint16_t overtemperature_cx10)
{
    device_.writeRegister(Registers::OV_THRESHOLD_MV, overvoltage_mv);
    device_.writeRegister(Registers::OT_THRESHOLD_CX10, overtemperature_cx10);
}

void DeviceController::pushMeasurement(double voltage_mv, double temperature_c)
{
    MeasurementData data{};
    data.voltage_mv = voltage_mv;
    data.temperature_c = temperature_c;

    device_.updateMeasurement(data);
}

MeasurementData DeviceController::readMeasurement() const
{
    return device_.getMeasurement();
}

DeviceSnapshot DeviceController::readSnapshot() const
{
    return device_.getSnapshot();
}