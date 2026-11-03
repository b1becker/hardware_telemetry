#include "collectors/SysInfoCollector.h"
#include <fstream>
#include <sstream>
#include <unistd.h>

std::string LinuxSysInfoCollector::readHostname() {
    char hostname[256] = {};

    if (gethostname(hostname, sizeof(hostname)) != 0) {
        return "";
    }

    hostname[sizeof(hostname) - 1] = '\0';

    return hostname;
}

std::optional<std::string>
LinuxSysInfoCollector::readFile(const std::string& path) {
    std::ifstream file(path);

    if (!file.is_open()) {
        return std::nullopt;
    }

    std::string value;

    if (!std::getline(file, value)) {
        return std::nullopt;
    }

    if (value.empty()) {
        return std::nullopt;
    }

    return value;
}

std::optional<double>
LinuxSysInfoCollector::readTotalMemory() {
    std::ifstream file("/proc/meminfo");

    if (!file.is_open()) {
        return std::nullopt;
    }

    std::string key;
    unsigned long long value;
    std::string unit;

    while (file >> key >> value >> unit) {
        if (key == "MemTotal:") {
            // /proc/meminfo reports kB.
            return static_cast<double>(value) / 1048576.0;
        }
    }

    return std::nullopt;
}

std::optional<long long> LinuxSysInfoCollector::readBootTime() {
    std::ifstream file("/proc/stat");

    if (!file.is_open()) {
        return std::nullopt;
    }

    std::string line;

    while (std::getline(file, line)) {
        std::istringstream stream(line);

        std::string key;
        long long bootTime;

        if (stream >> key >> bootTime && key == "btime") {
            return bootTime;
        }
    }

    return std::nullopt;
}

SystemInfo LinuxSysInfoCollector::collect() {
    SystemInfo info;

    info.hostname = readHostname();

    info.uuid = readFile(
        "/sys/class/dmi/id/product_uuid"
    );

    info.biosVersion = readFile(
        "/sys/class/dmi/id/bios_version"
    );

    info.totalMemoryGiB = readTotalMemory();

    info.bootTime = readBootTime();

    return info;
}