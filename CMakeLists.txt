cmake_minimum_required(VERSION 3.16)

project(HardwareTelemetry)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(hardware-telemetry
    main.cpp
    src/collectors/SysInfoCollector.cpp
)

target_include_directories(hardware-telemetry
    PRIVATE include
)