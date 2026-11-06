#include "EndpointValidator.hpp"
#include "TelemetryClient.hpp"

#include <iostream>
#include <memory>
#include <sstream>



// Overall functionality:
// How to run: <executable name> <IP address of Pi>
// Run the exectuable with 1 argument which is the IP address 
// of the PI

// What it does:
// Once the program is running, 
//     1. Input Redfish endpoint (ie "/redfish/v1/Systems/1")
//     2. "exit" halts the program
//     3. Malformed Inputs : notify user 

int main(int argc, char* argv[])
{
    if (argc != 2 || !isValidBaseUrl(argv[1]))
    {
        std::cerr << "Usage: telemetry-client http://HOST[:PORT]\n"
                << "Use a single HTTP host with no credentials, path, query, or "
                    "fragment.\n";
        return 1;
    }

    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
    {
        std::cerr << "Could not initialize libcurl.\n";
        return 1;
    }

    // We define a struct so that when our program terminates
    // (good or bad) we free the libcurl's resources
    struct CurlCleanup
    {
        ~CurlCleanup()
        {
            curl_global_cleanup();
        }
    } cleanup;

    
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(
        // Creates the HTTP handle
        // reference to some internal state in libcurl
        // that manages fields to acutally send GET
        curl_easy_init(),  
        curl_easy_cleanup
    );


    if (!curl)
    {
        std::cerr << "Could not create HTTP client.\n";
        return 1;
    }

    std::string baseUrl = argv[1];
    std::cout << "HardwareTelemetry Client\nTarget: " << baseUrl
            << "\nType 'connect http://HOST[:PORT]' to switch targets.\n"
            << "Type 'exit' to quit.\n";

    
    std::string path;
    while (true)
    {
        std::cout << "\nEnter Redfish endpoint or command:\n" << std::flush;
        if (!std::getline(std::cin, path) || path == "exit")
        {
            break;
        }

        // Commands are handled locally before endpoint validation or HTTP requests.
        std::istringstream commandLine(path);
        std::string command;
        commandLine >> command;
        if (command == "connect")
        {
            std::string newBaseUrl;
            std::string extraArgument;
            if (!(commandLine >> newBaseUrl) || (commandLine >> extraArgument) ||
                !isValidBaseUrl(newBaseUrl))
            {
                std::cerr << "Usage: connect http://HOST[:PORT]\n"
                          << "Target unchanged: " << baseUrl << '\n';
                continue;
            }

            // Select the target now; its reachability is checked by the next GET.
            baseUrl = newBaseUrl;
            std::cout << "Target: " << baseUrl << '\n';
            continue;
        }

        if (!isValidEndpoint(path))
        {
            std::cerr << "Unsupported endpoint. Supported: /redfish/v1/Systems/1\n";
            continue;
        }

        sendGetRequest(curl.get(), baseUrl, path);
    }

    return 0;
}
