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

namespace TransportCatalogue {

class TransportCatalogue {
public:
    struct Stop {
        std::string stop_name;
        Geography::Coordinates stop_coords;
    };

    struct MyHaser{
        size_t operator()(const std::pair<Stop*, Stop*>& p) const {
            size_t result = 0;
            std::hash<const void*> hasher;
            result += hasher(p.first);
            result += hasher(p.second) * 37;
            return result;
        }
    };

    struct Bus {
        std::string bus_name;
        std::vector<Stop*> bus_stops;
    };

    explicit TransportCatalogue(std::vector<Stop>& stops, std::vector<std::pair<std::string, std::deque<std::string>>>& buses, std::map<std::pair<std::string, std::string>, double>& real_distances);

    ResponseForBus GetBusResponse(std::string& bus_name);

    ResponseForStop GetStopResponse(std::string& stop_name);

private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, std::vector<Stop*>*> busname_to_bus_;
    std::unordered_map<std::pair<Stop*, Stop*>, double, MyHaser> real_distances_;
};
}
