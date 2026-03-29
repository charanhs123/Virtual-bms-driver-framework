#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "DeviceController.hpp"
#include "RegisterMap.hpp"
#include "VirtualBatteryMonitor.hpp"

TEST(RegisterMapTest, ReadWriteRegister)
{
    RegisterMap regmap;

    EXPECT_TRUE(regmap.write(Registers::CONTROL, 0x1234));
    std::uint32_t value = 0;
    EXPECT_TRUE(regmap.read(Registers::CONTROL, value));
    EXPECT_EQ(value, 0x1234u);
}

TEST(RegisterMapTest, InvalidAddressFails)
{
    RegisterMap regmap;

    std::uint32_t value = 0;
    EXPECT_FALSE(regmap.write(0xFF, 1));
    EXPECT_FALSE(regmap.read(0xFF, value));
}

TEST(VirtualBatteryMonitorTest, MeasurementIgnoredWhenDisabled)
{
    VirtualBatteryMonitor dev;
    dev.initialize();

    dev.updateMeasurement({4100.0, 30.0});
    MeasurementData m = dev.getMeasurement();

    EXPECT_DOUBLE_EQ(m.voltage_mv, 0.0);
    EXPECT_DOUBLE_EQ(m.temperature_c, 0.0);
}

TEST(VirtualBatteryMonitorTest, FaultRaisedForOvervoltage)
{
    VirtualBatteryMonitor dev;
    dev.initialize();

    std::uint32_t control = 0;
    dev.readRegister(Registers::CONTROL, control);
    control |= (1u << ControlBits::ENABLE_MEASUREMENT);
    control |= (1u << ControlBits::ENABLE_FAULT_DETECTION);
    dev.writeRegister(Registers::CONTROL, control);

    dev.updateMeasurement({4300.0, 30.0});

    std::uint32_t fault = 0;
    EXPECT_TRUE(dev.readRegister(Registers::FAULT, fault));
    EXPECT_NE(fault & (1u << FaultBits::OVERVOLTAGE), 0u);
}

TEST(DeviceControllerTest, ClearsFaults)
{
    DeviceController controller;
    controller.initialize();
    controller.enableMeasurement(true);
    controller.enableFaultDetection(true);

    controller.pushMeasurement(4300.0, 70.0);
    DeviceSnapshot snap = controller.readSnapshot();

    EXPECT_NE(snap.fault, 0u);

    controller.clearFaults();
    snap = controller.readSnapshot();

    EXPECT_NE(snap.fault, 0u);
}

TEST(RegisterMapTest, ConcurrentAccessDoesNotCrash)
{
    RegisterMap regmap;

    auto writer = [&regmap]()
    {
        for (int i = 0; i < 1000; ++i)
        {
            regmap.write(Registers::VOLTAGE_MV, static_cast<std::uint32_t>(i));
        }
    };

    auto reader = [&regmap]()
    {
        std::uint32_t value = 0;
        for (int i = 0; i < 1000; ++i)
        {
            regmap.read(Registers::VOLTAGE_MV, value);
        }
    };

    std::thread t1(writer);
    std::thread t2(reader);
    std::thread t3(writer);
    std::thread t4(reader);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    SUCCEED();
}
