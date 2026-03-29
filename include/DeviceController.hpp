#ifndef DEVICECONTROLLER_HPP
#define DEVICECONTROLLER_HPP

#include "DriverTypes.hpp"
#include "VirtualBatteryMonitor.hpp"

class DeviceController
{
public:
    DeviceController();

    void initialize();

    void enableMeasurement(bool enable);
    void enableFaultDetection(bool enable);
    void clearFaults();

    void configureThresholds(std::uint16_t overvoltage_mv,
                             std::uint16_t overtemperature_cx10);

    void pushMeasurement(double voltage_mv, double temperature_c);

    MeasurementData readMeasurement() const;
    DeviceSnapshot readSnapshot() const;

private:
    VirtualBatteryMonitor device_;
};

#endif