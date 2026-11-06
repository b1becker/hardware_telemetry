cmake_minimum_required(VERSION 3.16)
project(TelemetryClient LANGUAGES CXX)

find_package(CURL REQUIRED)

add_library(telemetry-client-core
    src/EndpointValidator.cpp
    src/TelemetryClient.cpp
)
target_compile_features(telemetry-client-core PUBLIC cxx_std_17)
target_include_directories(telemetry-client-core PUBLIC include)
target_link_libraries(telemetry-client-core PUBLIC CURL::libcurl)

add_executable(telemetry-client src/main.cpp)
target_link_libraries(telemetry-client PRIVATE telemetry-client-core)

include(CTest)
if(BUILD_TESTING)
    add_executable(validation-tests tests/ValidationTests.cpp)
    target_link_libraries(validation-tests PRIVATE telemetry-client-core)
    add_test(NAME validation COMMAND validation-tests)

    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(Python3_Interpreter_FOUND)
        add_test(NAME http-interactive
            COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration.py
                $<TARGET_FILE:telemetry-client>
        )
        set_tests_properties(http-interactive PROPERTIES TIMEOUT 45)
    endif()
endif()
