#pragma once

#include "collectors/VirtualSysInfoCollector.h"

class SystemService {
public:
    explicit SystemService(VirtualSysInfoCollector& collector);

    SystemInfo getSystemInfo();

private:
    VirtualSysInfoCollector& collector_;
};