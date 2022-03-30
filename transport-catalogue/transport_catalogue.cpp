#include "geo.h"
#include "transport_catalogue.h"

#include <string>
#include <set>
#include <vector>
#include <iterator>

using namespace std;

namespace TransportCatalogue {

typedef std::pair<std::string, std::string> stops_key;

size_t TransportCatalogue::MyHaser::operator()(const std::pair<Stop*, Stop*>& p) const {
    size_t result = 0;
    std::hash<const void*> hasher;
    result += hasher(p.first);
    result += hasher(p.second) * 37;
    return result;
}

TransportCatalogue::TransportCatalogue(std::vector<Stop>& stops, std::vector<std::pair<std::string, std::deque<std::string>>>& buses, std::map<stops_key, double>& real_distances) {
    InitStops(stops, real_distances);
    InitBuses(buses);
    InitStopnameToBuses();
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

    result.stops_count = (*iter_to_bus).second->bus_stops.size();

    result.unique_stops_count = (*iter_to_bus).second->unique_stops_count;

    result.route_real_lenght = (*iter_to_bus).second->route_real_lenght;

    result.curvature = (*iter_to_bus).second->route_real_lenght / (*iter_to_bus).second->route_unreal_lenght;

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

    if (stopname_to_buses_.find(stop_name) != stopname_to_buses_.end()) {
        result.bus_names = stopname_to_buses_[stop_name];
        result.is_buses_exists = true;
    } else {
        result.is_buses_exists = false;
    }

    return result;
}

void TransportCatalogue::InitStopnameToBuses() {
    for (auto& [name, bus] : busname_to_bus_) {
        for (auto& stop : bus->bus_stops) {
            stopname_to_buses_[stop->stop_name].insert(name);
        }
    }
}

void TransportCatalogue::InitStops(std::vector<Stop>& stops, std::map<stops_key, double>& real_distances) {
    for (auto& stop : stops) {
        stops_.push_back(stop);
        stopname_to_stop_[stops_.back().stop_name] = &(stops_.back());
    }
    for (auto& distance : real_distances) {
        real_distances_[{(*(stopname_to_stop_.find(distance.first.first))).second, (*(stopname_to_stop_.find(distance.first.second))).second}] = distance.second;
    }
}

void TransportCatalogue::InitBuses(std::vector<std::pair<std::string, std::deque<std::string>>>& buses) {
    for (auto& bus : buses) {
        buses_.push_back({bus.first, {}, 0, 0, 0});
    for (auto& stop_name : bus.second) {
        buses_.back().bus_stops.push_back( (*(stopname_to_stop_.find(stop_name))).second  );
    }
    busname_to_bus_[buses_.back().bus_name] = &(buses_.back());
    {
        set<string> tmp_set;
        for (auto& stop : buses_.back().bus_stops) {
            tmp_set.insert(stop->stop_name);
        }
        buses_.back().unique_stops_count = tmp_set.size();
    }
    vector<Stop*>::iterator it_to_stops_start = (buses_.back().bus_stops).begin();
    vector<Stop*>::iterator it_to_stops_start_2 = ++buses_.back().bus_stops.begin();
    for (size_t i = 0; i < buses_.back().bus_stops.size() - 1; ++i) {
        buses_.back().route_unreal_lenght += ComputeDistance((*it_to_stops_start)->stop_coords, (*it_to_stops_start_2)->stop_coords);
        ++it_to_stops_start;
        ++it_to_stops_start_2;
    }
    it_to_stops_start = buses_.back().bus_stops.begin();
    it_to_stops_start_2 = ++buses_.back().bus_stops.begin();;
    for (size_t i = 0; i < buses_.back().bus_stops.size() - 1; ++i) {
        if (real_distances_.find( {*it_to_stops_start, *it_to_stops_start_2} ) != real_distances_.end()) {
            buses_.back().route_real_lenght += real_distances_[{*it_to_stops_start, *it_to_stops_start_2}];
        } else {
            buses_.back().route_real_lenght += real_distances_[{*it_to_stops_start_2, *it_to_stops_start}];
        }
        ++it_to_stops_start;
        ++it_to_stops_start_2;
    }
}
}
}
