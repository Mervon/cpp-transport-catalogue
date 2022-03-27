#include "geo.h"
#include "transport_catalogue.h"

#include <string>
#include <set>
#include <vector>
#include <iterator>

using namespace std;

namespace TransportCatalogue {

TransportCatalogue::TransportCatalogue(std::vector<Stop>& stops, std::vector<std::pair<std::string, std::deque<std::string>>>& buses, std::map<std::pair<std::string, std::string>, double>& real_distances) {
    for (auto& stop : stops) {
        stops_.push_back(stop);
        stopname_to_stop_[stops_.back().stop_name] = &(stops_.back());
    }
    for (auto& bus : buses) {
        buses_.push_back({bus.first, {}});
        for (auto& stop_name : bus.second) {
            buses_.back().bus_stops.push_back( (*(stopname_to_stop_.find(stop_name))).second  );
        }
        busname_to_bus_[buses_.back().bus_name] = &(buses_.back().bus_stops);
    }
    for (auto& distance : real_distances) {
        real_distances_[{(*(stopname_to_stop_.find(distance.first.first))).second, (*(stopname_to_stop_.find(distance.first.second))).second}] = distance.second;
    }
}

ResponseForBus TransportCatalogue::GetBusResponse(string& bus_name) {
    ResponseForBus result;
    result.bus_name = bus_name;

    auto iter_to_bus = busname_to_bus_.find(bus_name);

    if (iter_to_bus != busname_to_bus_.end()) {
        result.is_bus_exist = true;
    } else {
        result.is_bus_exist = false;
        return result;
    }

    vector<Stop*>* bus_stops = (*iter_to_bus).second;

    result.stops_count = bus_stops->size();

    {
        set<Stop*> unique_stops(bus_stops->begin(), bus_stops->end());
        result.unique_stops = unique_stops.size();
    }
    double route_unreal_lenght = 0;
    vector<Stop*>::iterator it_to_stops_start = (*bus_stops).begin();
    vector<Stop*>::iterator it_to_stops_start_2 = ++(*bus_stops).begin();
    for (size_t i = 0; i < result.stops_count - 1; ++i) {
        route_unreal_lenght += ComputeDistance((*it_to_stops_start)->stop_coords, (*it_to_stops_start_2)->stop_coords);
        ++it_to_stops_start;
        ++it_to_stops_start_2;
    }

    it_to_stops_start = (*bus_stops).begin();
    it_to_stops_start_2 = ++(*bus_stops).begin();
    for (size_t i = 0; i < result.stops_count - 1; ++i) {

        if (real_distances_.find( {*it_to_stops_start, *it_to_stops_start_2} ) != real_distances_.end()) {
            result.route_real_lenght += real_distances_[{*it_to_stops_start, *it_to_stops_start_2}];
        } else {
            result.route_real_lenght += real_distances_[{*it_to_stops_start_2, *it_to_stops_start}];
        }
        ++it_to_stops_start;
        ++it_to_stops_start_2;
    }
    result.curvature = result.route_real_lenght / route_unreal_lenght;
    return result;
}

ResponseForStop TransportCatalogue::GetStopResponse(string& stop_name) {
    ResponseForStop result;
    result.stop_name = stop_name;

    if (stopname_to_stop_.find(stop_name) != stopname_to_stop_.end()) {
        result.is_stop_exists = true;
    } else {
        result.is_stop_exists = false;
        return result;
    }

    for (auto& [name, stops] : busname_to_bus_) {
        for (auto& stop : (*stops)) {
            if (stop_name == (*stop).stop_name) {
                result.bus_names.insert(name);
                break;
            }
        }
    }

    if (result.bus_names.size() == 0) {
        result.is_buses_exists = false;
    } else {
        result.is_buses_exists = true;
    }

    return result;
}
}
