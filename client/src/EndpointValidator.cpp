#include "EndpointValidator.hpp"
#include <algorithm>
#include <array>
#include <regex>


// We will be using this to store the current valid endpoints that we can send to the Pis
bool isValidEndpoint(const std::string& path)
{
    static const std::array<std::string, 1> supported{"/redfish/v1/Systems/1"};
    return std::find(supported.begin(), supported.end(), path) != supported.end();
}



// This function's primary job is to just validate that the BASE IP address is valid

// TODO: We may need another validation function in place of this
// It will depend on how we connect to the edge devices to request metrics
bool isValidBaseUrl(const std::string& url)
{
    // Restrict host names to ASCII DNS-style labels or dotted IPv4 addresses.
    // IPv6 and trailing slashes are intentionally outside this first slice.
    static const std::regex pattern(
        R"(^http://([A-Za-z0-9]([A-Za-z0-9-]*[A-Za-z0-9])?(\.[A-Za-z0-9]([A-Za-z0-9-]*[A-Za-z0-9])?)*)(:([0-9]{1,5}))?$)");
    std::smatch match;
    if (!std::regex_match(url, match, pattern))
    {
        return false;
    }
    const std::string host = match[1].str();
    if (host.size() > 253)
    {
        return false;
    }
    std::size_t start = 0;
    while (start < host.size())
    {
        const auto end = host.find('.', start);
        const auto length = (end == std::string::npos ? host.size() : end) - start;
        if (length > 63)
        {
            return false;
        }
        if (end == std::string::npos)
        {
            break;
        }
        start = end + 1;
    }
    if (match[6].matched)
    {
        const int port = std::stoi(match[6].str());
        if (port < 1 || port > 65535)
        {
            return false;
        }
    }
    return true;
}
