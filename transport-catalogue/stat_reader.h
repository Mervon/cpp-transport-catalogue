#pragma once

#include <vector>
#include <string>
#include <set>
#include <iostream>
#include <string_view>
#include <unordered_set>

namespace TransportCatalogue {

struct Response {
    std::string responses_for_stop;
    std::string responses_for_bus;
    bool is_for_bus;
};

struct ResponseForBus {
    std::string bus_name;
    size_t stops_count = 0;
    size_t unique_stops_count = 0;
    double route_real_lenght = 0;
    double curvature = 0;
    bool is_bus_exist;
};

struct ResponseForStop {
    std::string stop_name;
    std::set<std::string_view> bus_names;
    bool is_stop_exists;
    bool is_buses_exists;
};

namespace OutputFunctions{

void PrintStopResponse(ResponseForStop& r, std::ostream& os);

void PrintBusResponse(ResponseForBus& r, std::ostream& os);
}
}

