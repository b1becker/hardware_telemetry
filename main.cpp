
#include "crow.h"
#include "collectors/SysInfoCollector.h"

#include <iostream>
#include <csignal>

// Convert collected system information into JSON
crow::json::wvalue handleRequest(const SystemInfo& info)
{
    crow::json::wvalue response;

    response["@odata.id"] = "/redfish/v1/Systems/1";
    response["Id"] = "1";
    response["Name"] = "Edge System";

    response["HostName"] = info.hostname;

    if (info.uuid)
        response["UUID"] = *info.uuid;

    if (info.biosVersion)
        response["BiosVersion"] = *info.biosVersion;

    if (info.totalMemoryGiB) {
        crow::json::wvalue memory;
        memory["TotalSystemMemoryGiB"] = *info.totalMemoryGiB;
        response["MemorySummary"] = std::move(memory);
    }

    return response;
}

int main()
{
    crow::SimpleApp app;

    // Initialize Linux telemetry collector
    LinuxSysInfoCollector collector;

    // Register endpoint
    CROW_ROUTE(app, "/redfish/v1/Systems/1")
    ([&collector]() {

        std::cout << "Received system telemetry request"
                  << std::endl;

        SystemInfo info = collector.collect();

        return handleRequest(info);
    });

    std::cout << "====================================\n";
    std::cout << " HardwareTelemetry Daemon Starting\n";
    std::cout << "====================================\n";
    std::cout << "Listening address: 0.0.0.0\n";
    std::cout << "Listening port: 18080\n";
    std::cout << "Endpoint: /redfish/v1/Systems/1\n";

    // Start HTTP server
    app.port(18080).multithreaded().run();

    std::cout << "HardwareTelemetry daemon stopped.\n";

    return 0;
}
