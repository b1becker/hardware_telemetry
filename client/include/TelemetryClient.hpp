#pragma once
#include <curl/curl.h>
#include <string>

void sendGetRequest(CURL* curl, const std::string& baseUrl, const std::string& path);
