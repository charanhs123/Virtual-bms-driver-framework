cmake_minimum_required(VERSION 3.14)
project(virtual_bms_driver_framework VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_TESTS "Build unit tests" ON)

add_library(vbms_lib
    src/DriverTypes.cpp
    src/RegisterMap.cpp
    src/VirtualBatteryMonitor.cpp
    src/DeviceController.cpp
)

target_include_directories(vbms_lib PUBLIC include)

add_executable(vbms_demo
    src/main.cpp
)

target_link_libraries(vbms_demo PRIVATE vbms_lib)

if(ENABLE_TESTS)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(vbms_tests
        tests/test_main.cpp
    )

    target_link_libraries(vbms_tests
        PRIVATE
        vbms_lib
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(vbms_tests)
endif()