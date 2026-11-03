#pragma once

#include "collectors/VirtualSysInfoCollector.h"

class LinuxSysInfoCollector : public VirtualSysInfoCollector {
public:
    SystemInfo collect() override;

private:
    std::string readHostname();

    std::optional<std::string> readFile(
        const std::string& path
    );

    std::optional<std::string> readUUID();

    std::optional<std::string> readBiosVersion();

    std::optional<double> readTotalMemory();

    std::optional<long long> readBootTime();
};