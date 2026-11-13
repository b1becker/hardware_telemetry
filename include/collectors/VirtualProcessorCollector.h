#pragma once

#include "models/ProcessorInfo.h"
#include <vector>


class VirtualProcessorCollector {
public:
    virtual ~VirtualProcessorCollector() = default;

    virtual std::vector<ProcessorInfo> collect() = 0;
};