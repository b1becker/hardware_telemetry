#include "EndpointValidator.hpp"
#include <iostream>
#include <string>

int main()
{
    int failures = 0;
    const auto check = [&](bool ok, const std::string& input)
    {
        if (!ok)
        {
            std::cerr << "Failed: " << input << '\n';
            ++failures;
        }
    };
    check(isValidEndpoint("/redfish/v1/Systems/1"), "supported endpoint");
    for (const auto* input :
         {"", "exit", "/redfish/v1/Systems/999", "/random/path", "http://example.com",
          "/redfish/v1/Systems/1?x=1", "/redfish/v1/Systems/1#x", "/redfish/v1/Systems/../1",
          "/redfish/v1/Systems/1;ls", " /redfish/v1/Systems/1", "/redfish/v1/Systems/%31",
          "/redfish/v1/Systems/1\r"})
    {
        check(!isValidEndpoint(input), input);
    }
    for (const auto* input : {"http://localhost", "http://192.168.1.50:8080",
                              "http://pi.local:18080", "http://edge-node:65535"})
    {
        check(isValidBaseUrl(input), input);
    }
    for (const auto* input :
         {"", "https://pi", "http://", "http://pi/", "http://pi/path", "http://user:pass@pi",
          "http://pi?x=1", "http://pi#x", "http://pi:0", "http://pi:65536",
          "http://pi:9999999999999", "http://pi:abc", "http://pi\n", "http://pi host", "http://-pi",
          "http://pi..local", "http://pi\\example.com", "http://pi%2fexample.com"})
    {
        check(!isValidBaseUrl(input), input);
    }
    return failures ? 1 : 0;
}
