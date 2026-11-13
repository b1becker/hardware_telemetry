#pragma once

#include <optional>
#include <string>

struct ProcessorInfo {
    std::string id;

    std::optional<unsigned int> maxSpeedMHz;

    std::optional<unsigned int> operatingSpeedMHz;
};