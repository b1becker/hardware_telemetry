cmake_minimum_required(VERSION 3.16)

project(HardwareTelemetry LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Download Crow
include(FetchContent)

FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.2.1
)

FetchContent_MakeAvailable(Crow)

# Threading support
find_package(Threads REQUIRED)

# Build daemon
add_executable(hardware-telemetry
    main.cpp
    src/collectors/SysInfoCollector.cpp
)

# Include project headers
target_include_directories(hardware-telemetry PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link Crow and threading libraries
target_link_libraries(hardware-telemetry PRIVATE
    Crow::Crow
    Threads::Threads
)
