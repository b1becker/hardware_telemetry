#include "TelemetryClient.hpp"
#include "EndpointValidator.hpp"

#include <iostream>
#include <limits>

namespace
{
std::size_t appendBody(char* data, std::size_t size, std::size_t count, void* context) noexcept
{
    if (size && count > std::numeric_limits<std::size_t>::max() / size)
    {
        return 0;
    }
    const auto bytes = size * count;
    try
    {
        static_cast<std::string*>(context)->append(data, bytes);
        return bytes;
    }
    catch (...)
    {
        return 0; // Never let a C++ exception cross libcurl's C callback boundary.
    }
}
} // namespace

void sendGetRequest(CURL* curl, const std::string& baseUrl, const std::string& path)
{
    if (!curl || !isValidBaseUrl(baseUrl) || !isValidEndpoint(path))
    {
        std::cerr << "Invalid request configuration.\n";
        return;
    }

    curl_easy_reset(curl);
    const std::string url = baseUrl + path;
    std::string body;
    char error[CURL_ERROR_SIZE] = {};

    // Check every setting before starting the transfer.
    CURLcode setup = CURLE_OK;
    const auto set = [&](CURLoption option, auto value)
    {
        if (setup == CURLE_OK)
        {
            setup = curl_easy_setopt(curl, option, value);
        }
    };

    set(CURLOPT_URL, url.c_str());
    set(CURLOPT_HTTPGET, 1L);
    set(CURLOPT_WRITEFUNCTION, &appendBody);
    set(CURLOPT_WRITEDATA, &body);
    set(CURLOPT_ERRORBUFFER, error);
    set(CURLOPT_TIMEOUT, 10L);
    set(CURLOPT_CONNECTTIMEOUT, 5L);
    set(CURLOPT_FOLLOWLOCATION, 0L);
    set(CURLOPT_NOSIGNAL, 1L);

    if (setup != CURLE_OK)
    {
        std::cerr << "Request setup error: " << curl_easy_strerror(setup) << '\n';
        return;
    }

    const CURLcode result = curl_easy_perform(curl);
    if (result != CURLE_OK)
    {
        std::cerr << "Transport error: " << (error[0] ? error : curl_easy_strerror(result)) << '\n';
        return;
    }

    long status = 0;
    const auto infoResult = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    if (infoResult != CURLE_OK)
    {
        std::cerr << "Response status error: " << curl_easy_strerror(infoResult) << '\n';
        return;
    }

    std::cout << "HTTP Status: " << status << '\n';
    if (status >= 400)
    {
        std::cout << "HTTP error returned by server.\n";
    }
    std::cout << "\nResponse:\n";
    if (body.empty())
    {
        std::cout << "(empty response body)\n";
    }
    else
    {
        std::cout << body;
        if (body.back() != '\n')
        {
            std::cout << '\n';
        }
    }
}
