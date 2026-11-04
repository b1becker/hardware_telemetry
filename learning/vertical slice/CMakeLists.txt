
cmake_minimum_required(VERSION 3.16)

project(HardwareTelemetry LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required dependencies
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# Build the Redfish daemon
add_executable(hardwaretelemetry
    src/main.cpp
)

# Find project headers
target_include_directories(hardwaretelemetry PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Link dependencies
target_link_libraries(hardwaretelemetry PRIVATE
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)
