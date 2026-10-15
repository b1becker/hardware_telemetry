#include <curl/curl.h>
#include <iostream>
#include <string>

static size_t write_cb(char* p, size_t s, size_t n, void* out) {
    static_cast<std::string*>(out)->append(p, s * n);
    return s * n;
}

int main() {
    const std::string base = "http://localhost:8001/redfish/v1";
    const char* paths[] = {
        "/Chassis/Chassis-1/Thermal",
        "/Chassis/Chassis-1/Power",
        "/Systems/System-1",
        "/Systems/System-1/Memory",
    };

    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();
    if (!curl) return 1;

    for (const char* path : paths) {
        std::string body;
        curl_easy_setopt(curl, CURLOPT_URL, (base + path).c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);

        std::cout << "=== GET " << path << " ===\n";
        if (curl_easy_perform(curl) == CURLE_OK)
            std::cout << body << "\n\n";
        else
            std::cerr << "request failed\n";
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();
}