
#include "crow.h"
#include "collectors/SysInfoCollector.h"

#include <iostream>
#include <utility>

// Convert collected system information into a Redfish ComputerSystem
crow::json::wvalue handleRequest(const SystemInfo& info)
{
    crow::json::wvalue response;

    // ------------------------------------------------
    // Redfish resource metadata
    // ------------------------------------------------

    response["@odata.type"] = "#ComputerSystem.v1_XX_0.ComputerSystem";
    response["@odata.id"] = "/redfish/v1/Systems/1";

    response["Id"] = "1";
    response["Name"] = "Edge System";

    // ------------------------------------------------
    // ComputerSystem properties
    // ------------------------------------------------

    if (info.uuid)
        response["UUID"] = *info.uuid;

    // Add these when supported by SystemInfo:
    // response["Manufacturer"] = info.manufacturer;
    // response["Model"] = info.model;
    // response["SerialNumber"] = info.serialNumber;

    response["HostName"] = info.hostname;

    if (info.biosVersion)
        response["BiosVersion"] = *info.biosVersion;

    // ------------------------------------------------
    // Memory summary
    // ------------------------------------------------

    if (info.totalMemoryGiB)
    {
        crow::json::wvalue memory;

        memory["TotalSystemMemoryGiB"] = *info.totalMemoryGiB;

        response["MemorySummary"] = std::move(memory);
    }

    // ------------------------------------------------
    // Return Redfish resource
    // ------------------------------------------------

    return response;
}

int main()
{
    crow::SimpleApp app;

    // Initialize Linux telemetry collector
    LinuxSysInfoCollector collector;

    // Register ComputerSystem endpoint
    CROW_ROUTE(app, "/redfish/v1/Systems/1")
    ([&collector]()
    {
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
