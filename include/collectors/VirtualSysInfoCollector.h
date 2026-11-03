#pragma once

#include "models/SysInfo.h"

class VirtualSysInfoCollector {
public:
    virtual ~VirtualSysInfoCollector() = default;
    virtual SystemInfo collect() = 0;
};