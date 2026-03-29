#ifndef VIRTUALBATTERYMONITOR_HPP
#define VIRTUALBATTERYMONITOR_HPP

#include <cstdint>

#include "DriverTypes.hpp"
#include "RegisterMap.hpp"

class VirtualBatteryMonitor
{
public:
    VirtualBatteryMonitor();

    void initialize();

    bool writeRegister(std::uint8_t address, std::uint32_t value);
    bool readRegister(std::uint8_t address, std::uint32_t& value) const;

    void updateMeasurement(const MeasurementData& measurement);
    DeviceSnapshot getSnapshot() const;

    ThresholdConfig getThresholds() const;
    MeasurementData getMeasurement() const;

private:
    void evaluateFaults();
    void clearFaultsIfRequested();
    bool isMeasurementEnabled() const;
    bool isFaultDetectionEnabled() const;

    RegisterMap register_map_;
};

#endif