#include "collectors/ProcessorCollector.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <optional>
#include <algorithm>
#include <cctype>

std::optional<unsigned int>
LinuxProcessorCollector::readFrequencyMHz(
    const std::string& path
) {
    std::ifstream file(path);

    if (!file.is_open()) {
        return std::nullopt;
    }

    unsigned long long frequencyKHz;

    if (!(file >> frequencyKHz)) {
        return std::nullopt;
    }

    if (frequencyKHz == 0) {
        return std::nullopt;
    }

    // Linux reports CPU frequency in kHz.
    // Convert to MHz.
    return static_cast<unsigned int>(frequencyKHz / 1000);
}

bool LinuxProcessorCollector::isCpuOnline(
    const std::string& cpuPath
) {
    std::ifstream file(cpuPath + "/online");

    // CPU 0 may not have an online file.
    // If the file is absent, assume the CPU is online.
    if (!file.is_open()) {
        return true;
    }

    int online;

    if (!(file >> online)) {
        return false;
    }

    return online == 1;
}

std::vector<ProcessorInfo>
LinuxProcessorCollector::collect() {
    std::vector<ProcessorInfo> processors;

    const std::string cpuDirectory =
        "/sys/devices/system/cpu";

    std::error_code error;

    for (const auto& entry :
         std::filesystem::directory_iterator(cpuDirectory, error)) {

        if (error) {
            break;
        }

        if (!entry.is_directory()) {
            continue;
        }

        std::string cpuName = entry.path().filename().string();

        // Only process directories named cpu followed by a number.
        if (cpuName.rfind("cpu", 0) != 0) {
            continue;
        }

        std::string cpuId = cpuName.substr(3);

        if (cpuId.empty()) {
            continue;
        }

        if (!std::all_of(
                cpuId.begin(),
                cpuId.end(),
                [](unsigned char c) {
                    return std::isdigit(c);
                })) {
            continue;
        }

        std::string cpuPath = entry.path().string();

        if (!isCpuOnline(cpuPath)) {
            continue;
        }

        ProcessorInfo processor;

        processor.id = cpuId;

        processor.maxSpeedMHz = readFrequencyMHz(
            cpuPath + "/cpufreq/cpuinfo_max_freq"
        );

        processor.operatingSpeedMHz = readFrequencyMHz(
            cpuPath + "/cpufreq/scaling_cur_freq"
        );

        processors.push_back(processor);
    }

    return processors;
}