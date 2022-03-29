#pragma once

#include "geo.h"
#include "stat_reader.h"

#include <vector>
#include <set>
#include <string>
#include <string_view>
#include <deque>
#include <unordered_map>
#include <iostream>
#include <iomanip>
#include <map>
#include <unordered_set>

namespace TransportCatalogue {

typedef std::pair<std::string, std::string> stops_key;

class TransportCatalogue {
public:
    struct Stop {
        std::string stop_name;
        Geography::Coordinates stop_coords;
    };

    struct MyHaser{
        size_t operator()(const std::pair<Stop*, Stop*>& p) const;
    };

    struct Bus {
        std::string bus_name;
        std::vector<Stop*> bus_stops;
        int unique_stops_count = 0;
        double route_unreal_lenght = 0;
        double route_real_lenght = 0;
    };

    explicit TransportCatalogue(std::vector<Stop>& stops, std::vector<std::pair<std::string, std::deque<std::string>>>& buses, std::map<stops_key, double>& real_distances);

    ResponseForBus GetBusResponse(std::string& bus_name);

    ResponseForStop GetStopResponse(std::string& stop_name);

    void InitStopnameToBuses();

private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;
    std::unordered_map<std::pair<Stop*, Stop*>, double, MyHaser> real_distances_;
    std::unordered_map<std::string_view, std::set<std::string_view>> stopname_to_buses_;
};
}
