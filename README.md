# Virtual BMS Driver Framework (C++)

A Linux-based, embedded-inspired driver framework that simulates a **Battery Monitoring System (BMS) peripheral** using a register-mapped architecture.

This project models how real automotive embedded drivers interact with hardware devices, including control registers, measurement handling, and fault detection logic.

---

## Overview

The framework implements a **virtual battery monitoring IC** with:

* Register-based interface (similar to real hardware peripherals)
* Voltage and temperature measurement channels
* Configurable safety thresholds
* Fault detection (overvoltage, overtemperature)
* Status and diagnostic reporting
* Thread-safe register access

---

Modern embedded systems (especially automotive ECUs) rely heavily on:

* register-level device access
* deterministic fault handling
* hardware abstraction layers

This project demonstrates how to design such systems in **clean, modular C++**, while keeping the structure close to real embedded driver implementations.

---

##  System Architecture

```text
DeviceController
        ↓
VirtualBatteryMonitor (Device Logic)
        ↓
RegisterMap (Thread-safe Register Layer)
        ↓
Simulated Hardware State
```

### Key Components

###  RegisterMap

* Thread-safe register storage using `std::mutex`
* Read/write interface similar to memory-mapped I/O
* Bit-level manipulation (set/clear/check)

###  VirtualBatteryMonitor

* Core device logic
* Handles:

  * measurement updates
  * threshold checks
  * fault evaluation
  * status updates

###  DeviceController

* High-level API for interacting with the device
* Encapsulates:

  * control register operations
  * measurement injection
  * fault clearing
  * threshold configuration

---

##  Features

###  Register-based Device Model

* Control register
* Status register
* Fault register
* Measurement registers
* Threshold configuration registers

###  Fault Detection Logic

* Overvoltage detection
* Overtemperature detection
* Status flag updates
* Fault re-evaluation based on current conditions

###  Thread Safety

* Concurrent access handled via mutex-protected register map
* Verified through multithreaded unit tests

###  Realistic Fault Behavior

Faults are **condition-based**, meaning:

> Clearing faults does not remove them unless the underlying unsafe condition is resolved.

This mirrors real-world embedded safety systems.

---

## Unit Testing

Test coverage includes:

* Register read/write validation
* Invalid address handling
* Fault triggering logic
* Fault clearing behavior
* Multithreaded access safety

Run tests:

```bash
ctest --output-on-failure
```

---

## Running the Demo

```bash
./vbms_demo
```

Example output:

```text
[INFO] Writing measurement -> Voltage: 4255.0 mV, Temperature: 43.1 C
STATUS : FAULT_ACTIVE
FAULT  : OVERVOLTAGE
```

---

