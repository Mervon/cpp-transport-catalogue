#pragma once

#include "geo.h"
#include "domain.h"

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

    struct MyHaser{
        size_t operator()(const std::pair<Stop*, Stop*>& p) const;
    };

    TransportCatalogue() = default;

    TransportCatalogue(std::vector<Stop>& stops, std::vector<std::pair<std::string, std::deque<std::string>>>& buses, std::map<stops_key, double>& real_distances, std::map<std::string, AditionalInfo>& aditional_info);

    ResponseForBus GetBusResponse(std::string& bus_name);

    ResponseForStop GetStopResponse(std::string& stop_name);

    void InitStopnameToBuses();

    void InitStops(std::vector<Stop>& stops, std::map<stops_key, double>& real_distances);

    void InitBuses(std::vector<std::pair<std::string, std::deque<std::string>>>& buses);

    std::vector<geo::Coordinates> GetAllCoords();

    std::deque<Bus> GetAllBusesSortedByName();

    std::deque<Stop> GetAllStopsSortedByName();

    std::map<std::string, AditionalInfo>& GetAditionalInfo();

    Stop* GetStopByName(const std::string& name);

    bool HaveBuses(const std::string& s);

private:

    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;
    std::unordered_map<std::pair<Stop*, Stop*>, double, MyHaser> real_distances_;
    std::unordered_map<std::string_view, std::set<std::string_view>> stopname_to_buses_;
    std::map<std::string, AditionalInfo> aditional_info_;
};
}
