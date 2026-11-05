
#include "include/crow.h"
#include "collectors/SysInfoCollector.h"

#include <iostream>
#include <string>

// Convert collected system information into a Redfish-style JSON response
crow::json::wvalue handleRequest(
    const SystemInfo& info
) {
    crow::json::wvalue response;

    // Static Redfish resource information
    response["@odata.id"] = "/redfish/v1/Systems/1";
    response["Id"] = "1";
    response["Name"] = "Edge System";

    // Values collected from Linux
    response["HostName"] = info.hostname;

    if (info.uuid) {
        response["UUID"] = *info.uuid;
    }

    if (info.biosVersion) {
        response["BiosVersion"] = *info.biosVersion;
    }

    if (info.totalMemoryGiB) {
        crow::json::wvalue memorySummary;
        memorySummary["TotalSystemMemoryGiB"] =
            *info.totalMemoryGiB;

        response["MemorySummary"] = std::move(memorySummary);
    }

    // Boot time is currently Unix seconds.
    // Convert it to the required Redfish date-time format
    // in a later normalization step before exposing LastResetTime.

    return response;
}

int main()
{
    crow::SimpleApp app;

    // Create the collector once when the daemon starts
    LinuxSysInfoCollector collector;

    // Register the Redfish System resource route
    CROW_ROUTE(app, "/redfish/v1/Systems/1")
    ([&collector]() {

        // Collect current system information
        SystemInfo info = collector.collect();

        // Return the JSON response
        return handleRequest(info);
    });

    std::cout << "HardwareTelemetry daemon listening on port 18080..."
              << std::endl;

    // Crow manages the HTTP server and incoming connections
    app.port(18080).multithreaded().run();

    return 0;
}
