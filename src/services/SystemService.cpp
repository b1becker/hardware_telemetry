#include "services/SystemService.h"

SystemService::SystemService(VirtualSysInfoCollector& collector)
    : collector_(collector) {}

SystemInfo SystemService::getSystemInfo() {
    return collector_.collect();
}