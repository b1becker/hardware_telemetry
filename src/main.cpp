#include <iostream>
#include "collectors/SysInfoCollector.h"

int main() {

    LinuxSysInfoCollector collector;

    SystemInfo info = collector.collect();

    std::cout << "Hostname: " << info.hostname << std::endl;

    if (info.totalMemoryGiB) {
        std::cout << "Memory: "
                  << *info.totalMemoryGiB << " GiB"
                  << std::endl;
    }

    if (info.bootTime) {
        std::cout << "Boot Time: "
                  << *info.bootTime << std::endl;
    }

    if (info.uuid) {
        std::cout << "UUID: " << *info.uuid << std::endl;
    }

    if (info.biosVersion) {
        std::cout << "BIOS: "
                  << *info.biosVersion << std::endl;
    }

    return 0;
}