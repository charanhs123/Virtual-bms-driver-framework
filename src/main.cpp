#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

#include "DeviceController.hpp"
#include "DriverTypes.hpp"

namespace
{
void printSnapshot(const DeviceSnapshot& snap)
{
    std::cout << "----------------------------------------\n";
    std::cout << "CONTROL      : 0x" << std::hex << snap.control << std::dec << '\n';
    std::cout << "STATUS       : 0x" << std::hex << snap.status << std::dec
              << " [" << statusRegisterToString(snap.status) << "]\n";
    std::cout << "FAULT        : 0x" << std::hex << snap.fault << std::dec
              << " [" << faultRegisterToString(snap.fault) << "]\n";
    std::cout << "VOLTAGE      : " << snap.voltage_mv << " mV\n";
    std::cout << "TEMPERATURE  : " << static_cast<double>(snap.temperature_cx10) / 10.0 << " C\n";
    std::cout << "OV THRESHOLD : " << snap.ov_threshold_mv << " mV\n";
    std::cout << "OT THRESHOLD : " << static_cast<double>(snap.ot_threshold_cx10) / 10.0 << " C\n";
    std::cout << "DEVICE ID    : 0x" << std::hex << snap.device_id << std::dec << '\n';
}
}

int main()
{
    DeviceController controller;
    controller.initialize();

    controller.enableMeasurement(true);
    controller.enableFaultDetection(true);
    controller.configureThresholds(4200, 550); // 4.2V, 55.0C

    const std::vector<MeasurementData> samples = {
        {3680.0, 28.5},
        {3890.0, 31.2},
        {4150.0, 42.7},
        {4255.0, 43.1},
        {4180.0, 58.4}
    };

    std::cout << std::fixed << std::setprecision(1);
    std::cout << "[INFO] Virtual BMS Driver Demo Start\n";

    for (const auto& sample : samples)
    {
        std::cout << "\n[INFO] Writing measurement -> Voltage: "
                  << sample.voltage_mv << " mV, Temperature: "
                  << sample.temperature_c << " C\n";

        controller.pushMeasurement(sample.voltage_mv, sample.temperature_c);

        const DeviceSnapshot snap = controller.readSnapshot();
        printSnapshot(snap);

        if (snap.fault != 0u)
        {
            std::cout << "[WARN] Fault detected: "
                      << faultRegisterToString(snap.fault) << '\n';
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    std::cout << "\n[INFO] Clearing faults\n";
    controller.clearFaults();

    printSnapshot(controller.readSnapshot());

    std::cout << "\n[INFO] Demo completed\n";
    return 0;
}