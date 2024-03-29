#include "geo.h"
#include "transport_catalogue.h"
#include "svg.h"

#include <algorithm>
#include <string>
#include <set>
#include <vector>
#include <iterator>


#include <iostream>

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

TransportCatalogue::TransportCatalogue(std::vector<Stop>& stops, std::vector<std::pair<std::string, std::deque<std::string>>>& buses, std::map<stops_key, double>& real_distances, map<string, AditionalInfo>& aditional_info) : aditional_info_(aditional_info) {

    InitStops(stops, real_distances);

    InitBuses(buses);

    InitStopnameToBuses();
}

std::optional<graph::Router<double>::RouteInfo> TransportCatalogue::GetRouteResponse(std::string stop_name_1, std::string stop_name_2) {
	
    std::optional<graph::Router<double>::RouteInfo> result = (*(transport_router_.GetRouter())).BuildRoute(stopname_to_vertex_id_[stop_name_1], stopname_to_vertex_id_[stop_name_2]);
    
    return result;
}

void TransportCatalogue::ProcessGraph(const RoutingSettings& routing_settings) {
    graph::DirectedWeightedGraph<double> graph(stops_.size());
    for (auto& [name, bus] : busname_to_bus_) {
        std::vector<Stop*>& bus_stops = bus->bus_stops;
        if (bus_stops.size() < 2) {
            return;
        }
        for (size_t i = 0; i < bus_stops.size(); ++i) {
            double current_lenght = routing_settings.bus_wait_time_;
            int span_count = 1;
            size_t from = stopname_to_vertex_id_[bus_stops[i]->stop_name];
            for (size_t j = i + 1; j < bus_stops.size(); ++j) {
                size_t to = stopname_to_vertex_id_[bus_stops[j]->stop_name];
                auto candidat = real_distances_[{bus_stops[j - 1], bus_stops[j]}];
                if (candidat == 0) {
                    candidat = real_distances_[{bus_stops[j], bus_stops[j - 1]}];
                }
                current_lenght += candidat / 1000.0 / routing_settings.bus_velocity * 60.0;
                graph.AddEdge(graph::Edge<double>{from, to, current_lenght, name, span_count++});
            }
        }
    }
    graph_ = move(graph);

    transport_router_.ProcessRouter(graph_);
}



::ResponseForBus TransportCatalogue::GetBusResponse(const std::string& bus_name) const {
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

::ResponseForStop TransportCatalogue::GetStopResponse(std::string& stop_name) {
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
    size_t unique_vertex_id = 0;
    for (auto& stop : stops) {
        stops_.push_back(stop);
        stopname_to_stop_[stops_.back().stop_name] = &(stops_.back());
        stopname_to_vertex_id_[stops_.back().stop_name] = unique_vertex_id;
        vertex_id_to_stopname_[unique_vertex_id++] = stops_.back().stop_name;
    }
    for (auto& distance : real_distances) {
        real_distances_[{(*(stopname_to_stop_.find(distance.first.first))).second, (*(stopname_to_stop_.find(distance.first.second))).second}] = distance.second;
    }
}

void TransportCatalogue::InitBuses(std::vector<std::pair<std::string, std::deque<std::string>>>& buses) {
    for (auto& bus : buses) {
        buses_.push_back({svg::NoneColor, bus.first, {}, 0, 0, 0});
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

vector<geo::Coordinates> TransportCatalogue::GetAllCoords() {
    vector<geo::Coordinates> result;
    for (auto& stop : stops_) {
        if (stopname_to_buses_.find(stop.stop_name) != stopname_to_buses_.end()) {
            if (stopname_to_buses_.at(stop.stop_name).size() != 0) {
                result.push_back(stop.stop_coords);
            }
        }
    }
    return result;
}

std::deque<Bus> TransportCatalogue::GetAllBusesSortedByName() const {
    std::deque<Bus> buses_copy = buses_;
    sort(buses_copy.begin(), buses_copy.end(), [](const Bus& lhs, const Bus& rhs){
         return lhs.bus_name < rhs.bus_name;});
    return buses_copy;
}

std::map<std::string, AditionalInfo>& TransportCatalogue::GetAditionalInfo() {
    return aditional_info_;
}

Stop* TransportCatalogue::GetStopByName(const std::string& name) {
    return stopname_to_stop_[name];
}

std::deque<Stop> TransportCatalogue::GetAllStopsSortedByName() const {
    std::deque<Stop> stops_copy = stops_;
    sort(stops_copy.begin(), stops_copy.end(), [](const Stop& lhs, const Stop& rhs){
         return lhs.stop_name < rhs.stop_name;});
    return stops_copy;
}

const std::unordered_map<std::string_view, size_t>& TransportCatalogue::GetStopNameToVertexId() {
    return stopname_to_vertex_id_;
}

const std::unordered_map<size_t, std::string_view>& TransportCatalogue::GetVertexIdToStopName() {
    return vertex_id_to_stopname_;
}

bool TransportCatalogue::HaveBuses(const std::string& s) {
    if (stopname_to_buses_.find(s) != stopname_to_buses_.end()) {
        if (stopname_to_buses_.at(s).size() != 0) {
            return true;
        }
    }
    return false;
}

const std::set<std::string_view>& TransportCatalogue::GetBusesByStop(const std::string_view& stop_name) const {
    return stopname_to_buses_.at(stop_name);
}

const graph::DirectedWeightedGraph<double>& TransportCatalogue::GetGraph() {
    return graph_;
}

const std::deque<Bus>& TransportCatalogue::GetAllBuses() const {
	return buses_;
}

const std::deque<Stop>& TransportCatalogue::GetAllStops() const {
	return stops_;
}

void TransportCatalogue::SetStops(std::deque<Stop>& stops) {
	stops_ = std::move(stops);
}

void TransportCatalogue::SetBuses(std::deque<Bus>& buses) {
	buses_ = std::move(buses);
}

void TransportCatalogue::SetStopname_to_stop(std::unordered_map<std::string_view, Stop*>& stopname_to_stop) {
	stopname_to_stop_ = std::move(stopname_to_stop);
}

void TransportCatalogue::Setbusname_to_bus(std::unordered_map<std::string_view, Bus*>& busname_to_bus) {
	busname_to_bus_ = std::move(busname_to_bus);
}

void TransportCatalogue::SetAditionalinfo(std::map<std::string, AditionalInfo>& aditional_info) {
	aditional_info_ = std::move(aditional_info);
}

void TransportCatalogue::SetGraph(graph::DirectedWeightedGraph<double>& graph) {
	graph_ = std::move(graph);
	transport_router_.ProcessRouter(graph_);
}

const std::unordered_map<std::string_view, Bus*>& TransportCatalogue::GetBusnameToBus() const {
	return busname_to_bus_;
}

const std::unordered_map<std::string_view, Stop*>& TransportCatalogue::GetStopNameToStop() const {
	return stopname_to_stop_;
}

void TransportCatalogue::SetStopNameToVertexId(std::unordered_map<std::string_view, size_t>& stopname_to_vertex_id, std::unordered_map<size_t, std::string_view> vertex_id_to_stopname) {
	stopname_to_vertex_id_ = std::move(stopname_to_vertex_id);
	vertex_id_to_stopname_ = std::move(vertex_id_to_stopname);
}

}