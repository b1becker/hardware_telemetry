#pragma once

#include <optional>
#include <string>

struct SystemInfo {
    std::string hostname;

    std::optional<std::string> uuid;
    std::optional<std::string> biosVersion;

    std::optional<double> totalMemoryGiB;

    std::optional<long long> bootTime;
};