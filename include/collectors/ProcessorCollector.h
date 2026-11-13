#pragma once

#include "collectors/VirtualProcessorCollector.h"

class LinuxProcessorCollector : public VirtualProcessorCollector {
public:
    std::vector<ProcessorInfo> collect() override;

private:
    std::optional<unsigned int> readFrequencyMHz(
        const std::string& path
    );

    bool isCpuOnline(
        const std::string& cpuPath
    );
};