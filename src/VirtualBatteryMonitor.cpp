#include "VirtualBatteryMonitor.hpp"

#include "BitUtils.hpp"

namespace
{
constexpr std::uint16_t DEFAULT_OV_THRESHOLD_MV = 4200;
constexpr std::uint16_t DEFAULT_OT_THRESHOLD_CX10 = 600; // 60.0 C
}

VirtualBatteryMonitor::VirtualBatteryMonitor()
{
    initialize();
}

void VirtualBatteryMonitor::initialize()
{
    register_map_.reset();

    register_map_.write(Registers::OV_THRESHOLD_MV, DEFAULT_OV_THRESHOLD_MV);
    register_map_.write(Registers::OT_THRESHOLD_CX10, DEFAULT_OT_THRESHOLD_CX10);

    register_map_.setBit(Registers::STATUS, StatusBits::DEVICE_READY);
}

bool VirtualBatteryMonitor::writeRegister(std::uint8_t address, std::uint32_t value)
{
    const bool ok = register_map_.write(address, value);
    if (!ok)
    {
        return false;
    }

    clearFaultsIfRequested();

    if (address == Registers::VOLTAGE_MV || address == Registers::TEMPERATURE_CX10 ||
        address == Registers::OV_THRESHOLD_MV || address == Registers::OT_THRESHOLD_CX10 ||
        address == Registers::CONTROL)
    {
        evaluateFaults();
    }

    return true;
}

bool VirtualBatteryMonitor::readRegister(std::uint8_t address, std::uint32_t& value) const
{
    return register_map_.read(address, value);
}

void VirtualBatteryMonitor::updateMeasurement(const MeasurementData& measurement)
{
    if (!isMeasurementEnabled())
    {
        return;
    }

    const std::uint32_t voltage_mv =
        static_cast<std::uint32_t>((measurement.voltage_mv < 0.0) ? 0.0 : measurement.voltage_mv);

    const double temp_cx10_double = measurement.temperature_c * 10.0;
    const std::uint32_t temp_cx10 =
        static_cast<std::uint32_t>((temp_cx10_double < 0.0) ? 0.0 : temp_cx10_double);

    register_map_.write(Registers::VOLTAGE_MV, BitUtils::clampToU16(voltage_mv));
    register_map_.write(Registers::TEMPERATURE_CX10, BitUtils::clampToU16(temp_cx10));
    register_map_.setBit(Registers::STATUS, StatusBits::MEASUREMENT_VALID);

    evaluateFaults();
}

DeviceSnapshot VirtualBatteryMonitor::getSnapshot() const
{
    return register_map_.snapshot();
}

ThresholdConfig VirtualBatteryMonitor::getThresholds() const
{
    DeviceSnapshot snap = register_map_.snapshot();

    ThresholdConfig cfg{};
    cfg.overvoltage_mv = static_cast<std::uint16_t>(snap.ov_threshold_mv);
    cfg.overtemperature_cx10 = static_cast<std::uint16_t>(snap.ot_threshold_cx10);

    return cfg;
}

MeasurementData VirtualBatteryMonitor::getMeasurement() const
{
    DeviceSnapshot snap = register_map_.snapshot();

    MeasurementData data{};
    data.voltage_mv = static_cast<double>(snap.voltage_mv);
    data.temperature_c = static_cast<double>(snap.temperature_cx10) / 10.0;

    return data;
}

bool VirtualBatteryMonitor::isMeasurementEnabled() const
{
    bool result = false;
    register_map_.isBitSet(Registers::CONTROL, ControlBits::ENABLE_MEASUREMENT, result);
    return result;
}

bool VirtualBatteryMonitor::isFaultDetectionEnabled() const
{
    bool result = false;
    register_map_.isBitSet(Registers::CONTROL, ControlBits::ENABLE_FAULT_DETECTION, result);
    return result;
}

void VirtualBatteryMonitor::clearFaultsIfRequested()
{
    bool clear_requested = false;
    register_map_.isBitSet(Registers::CONTROL, ControlBits::CLEAR_FAULTS, clear_requested);

    if (!clear_requested)
    {
        return;
    }

    register_map_.write(Registers::FAULT, 0u);
    register_map_.clearBit(Registers::STATUS, StatusBits::FAULT_ACTIVE);
    register_map_.clearBit(Registers::CONTROL, ControlBits::CLEAR_FAULTS);
}

void VirtualBatteryMonitor::evaluateFaults()
{
    clearFaultsIfRequested();

    if (!isFaultDetectionEnabled())
    {
        register_map_.write(Registers::FAULT, 0u);
        register_map_.clearBit(Registers::STATUS, StatusBits::FAULT_ACTIVE);
        return;
    }

    DeviceSnapshot snap = register_map_.snapshot();

    std::uint32_t new_fault = 0u;

    if (snap.voltage_mv > snap.ov_threshold_mv)
    {
        BitUtils::setBit(new_fault, FaultBits::OVERVOLTAGE);
    }

    if (snap.temperature_cx10 > snap.ot_threshold_cx10)
    {
        BitUtils::setBit(new_fault, FaultBits::OVERTEMPERATURE);
    }

    register_map_.write(Registers::FAULT, new_fault);

    if (new_fault != 0u)
    {
        register_map_.setBit(Registers::STATUS, StatusBits::FAULT_ACTIVE);
    }
    else
    {
        register_map_.clearBit(Registers::STATUS, StatusBits::FAULT_ACTIVE);
    }
}