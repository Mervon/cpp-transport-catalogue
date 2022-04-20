#pragma once

#include "geo.h"
#include "svg.h"

#include <vector>
#include <string>
#include <set>

struct Stop {
    std::string stop_name;
    geo::Coordinates stop_coords;
};

struct Bus {
    svg::Color color;
    std::string bus_name;
    std::vector<Stop*> bus_stops;
    int unique_stops_count = 0;
    double route_unreal_lenght = 0;
    double route_real_lenght = 0;
};

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

struct AditionalInfo{
    std::string last_stop;
};
