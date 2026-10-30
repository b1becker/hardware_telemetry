
#include <charconv>
#include <chrono>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>
#include <string_view>
#include <system_error>

// push fodder

std::string utc_timestamp()
{
    using namespace std::chrono;

    const auto now = system_clock::now();
    const auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);    
    const auto millis = duration_cast<milliseconds>(now - seconds).count();

    const std::time_t time =
        system_clock::to_time_t(seconds);

    std::tm utc{};
    if (gmtime_r(&time, &utc) == nullptr) {
        return {};
    }

    char buffer[32]{};
    if (std::strftime(buffer, sizeof(buffer),
                      "%Y-%m-%dT%H:%M:%S", &utc) == 0) {
        return {};
    }

    std::ostringstream out;
    out << buffer << '.'
        << std::setw(3) << std::setfill('0') << millis
        << 'Z';

    return out.str();
}

int main(int argc, char* argv[])
{
    const std::string path =
        argc > 1
            ? argv[1]
            : "/sys/class/thermal/thermal_zone0/temp";

    std::ifstream input(path);
    if (!input) {
        std::cerr << "Cannot read temperature source: "
                  << path << '\n';
        return 1;
    }

    std::string raw;
    input >> raw;

    long long millidegrees = 0;
    const char* begin = raw.data();
    const char* end = begin + raw.size();

    const auto result =
        std::from_chars(begin, end, millidegrees);

    if (raw.empty() || result.ec != std::errc{} ||
        result.ptr != end) {
        std::cerr << "Invalid temperature reading\n";
        return 1;
    }

    const double celsius =
        static_cast<double>(millidegrees) / 1000.0;

    // Reject implausible values and obvious sensor errors.
    if (!std::isfinite(celsius) ||
        celsius < -40.0 || celsius > 150.0) {
        std::cerr << "Temperature outside accepted range\n";
        return 1;
    }

    const std::string timestamp = utc_timestamp();
    if (timestamp.empty()) {
        std::cerr << "Could not obtain timestamp\n";
        return 1;
    }

    std::cout << std::fixed << std::setprecision(2)
              << "{\n"
              << "  \"sensor_id\": \"CPU_Temperature\",\n"
              << "  \"reading\": " << celsius << ",\n"
              << "  \"unit\": \"Cel\",\n"
              << "  \"timestamp\": \"" << timestamp << "\",\n"
              << "  \"status\": \"OK\"\n"
              << "}\n";

    return 0;
}
