#include "crow.h"
#include "collectors/SysInfoCollector.h"
#include "collectors/ProcessorCollector.h"

#include <iostream>
#include <utility>
#include <algorithm>
#include <string>
#include <vector>

// Convert collected system information into a Redfish ComputerSystem
crow::json::wvalue handleRequest(const SystemInfo& info)
{
    crow::json::wvalue response;

    // Redfish resource metadata
    response["@odata.type"] = "#ComputerSystem.v1_XX_0.ComputerSystem";
    response["@odata.id"] = "/redfish/v1/Systems/1";

    response["Id"] = "1";
    response["Name"] = "Edge System";

    if (info.uuid)
        response["UUID"] = *info.uuid;

    // response["Manufacturer"] = info.manufacturer;
    // response["Model"] = info.model;
    // response["SerialNumber"] = info.serialNumber;

    response["HostName"] = info.hostname;

    if (info.biosVersion)
        response["BiosVersion"] = *info.biosVersion;

    // Memory summary
    if (info.totalMemoryGiB)
    {
        crow::json::wvalue memory;

        memory["TotalSystemMemoryGiB"] = *info.totalMemoryGiB;

        response["MemorySummary"] = std::move(memory);
    }

    return response;
}

// Convert an individual ProcessorInfo into a Redfish Processor
crow::json::wvalue handleProcessorRequest(
    const ProcessorInfo& info)
{
    crow::json::wvalue response;

    const std::string processorPath =
        "/redfish/v1/Systems/1/Processors/" + info.id;

    // Redfish resource metadata
    response["@odata.type"] = "#Processor.v1_XX_0.Processor";
    response["@odata.id"] = processorPath;

    response["Id"] = info.id;
    response["Name"] = "CPU " + info.id;
    response["ProcessorType"] = "CPU";

    // Processor frequency properties
    if (info.maxSpeedMHz)
        response["MaxSpeedMHz"] = *info.maxSpeedMHz;

    if (info.operatingSpeedMHz)
        response["OperatingSpeedMHz"] = *info.operatingSpeedMHz;

    return response;
}

// Convert collected processors into a Redfish ProcessorCollection
crow::json::wvalue handleProcessorCollection(
    const std::vector<ProcessorInfo>& processors)
{
    crow::json::wvalue response;

    response["@odata.type"] =
        "#ProcessorCollection.ProcessorCollection";

    response["@odata.id"] =
        "/redfish/v1/Systems/1/Processors";

    response["Name"] = "Processor Collection";

    response["Members@odata.count"] =
        processors.size();

    crow::json::wvalue members;

    for (std::size_t i = 0; i < processors.size(); ++i)
    {
        crow::json::wvalue member;

        member["@odata.id"] =
            "/redfish/v1/Systems/1/Processors/"
            + processors[i].id;

        members[i] = std::move(member);
    }

    response["Members"] = std::move(members);

    return response;
}

int main()
{
    crow::SimpleApp app;

    // Initialize Linux telemetry collectors
    LinuxSysInfoCollector sysInfoCollector;
    LinuxProcessorCollector processorCollector;

    // System endpoint
    CROW_ROUTE(app, "/redfish/v1/Systems/1")
    ([&sysInfoCollector]()
    {
        std::cout << "Received system telemetry request"
                  << std::endl;

        SystemInfo info = sysInfoCollector.collect();

        return handleRequest(info);
    });

    // General Processor collection endpoint
    CROW_ROUTE(app, "/redfish/v1/Systems/1/Processors")
    ([&processorCollector]()
    {
        std::cout << "Received processor collection request"
                  << std::endl;

        std::vector<ProcessorInfo> processors =
            processorCollector.collect();

        return handleProcessorCollection(processors);
    });

    // Individual processor endpoint
    CROW_ROUTE(app, "/redfish/v1/Systems/1/Processors/<string>")
    ([&processorCollector](const std::string& processorId)
    {
        std::cout << "Received processor request for CPU "
                  << processorId << std::endl;

        std::vector<ProcessorInfo> processors =
            processorCollector.collect();

        auto it = std::find_if(
            processors.begin(),
            processors.end(),
            [&processorId](const ProcessorInfo& processor)
            {
                return processor.id == processorId;
            }
        );

        if (it == processors.end())
        {
            crow::json::wvalue error;

            error["error"] = "Processor not found";

            return crow::response(
                404,
                error
            );
        }

        return crow::response(
            handleProcessorRequest(*it)
        );
    });

    // Startup prints
    std::cout << " HardwareTelemetry Daemon Starting\n";

    std::cout << "Listening address: 0.0.0.0\n";
    std::cout << "Listening port: 18080\n";

    // Start HTTP server
    app.port(18080).multithreaded().run();

    std::cout << "HardwareTelemetry daemon stopped.\n";

    return 0;
}
