# Virtual BMS Driver Framework

A Linux user-space, embedded-inspired virtual driver framework for a battery monitoring device.

## Features

- Register-mapped device abstraction
- Control, status, and fault registers
- Voltage and temperature monitoring
- Overvoltage and overtemperature fault detection
- Thread-safe register access
- Unit tests with GoogleTest
- CMake build system

## Motivation

This project demonstrates how low-level embedded driver concepts can be modeled in a clean and testable C++ design.

The framework simulates a battery monitoring peripheral typically found in automotive Battery Management Systems (BMS), including:

- register access
- threshold configuration
- fault handling
- device state reporting

## Build

```bash
mkdir build
cd build
cmake ..
cmake --build .