#pragma once

#include "geo.h"
#include "domain.h"
#include "graph.h"
#include "transport_router.h"

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

    void ProcessGraph(const RoutingSettings& routing_settings);

    TransportCatalogue() = default;

    TransportCatalogue(std::vector<Stop>& stops, std::vector<std::pair<std::string, std::deque<std::string>>>& buses, std::map<stops_key, double>& real_distances, std::map<std::string, AditionalInfo>& aditional_info);

    ResponseForBus GetBusResponse(const std::string& bus_name) const ;

    ResponseForStop GetStopResponse(std::string& stop_name);

    void InitStopnameToBuses();

    void InitStops(std::vector<Stop>& stops, std::map<stops_key, double>& real_distances);

    void InitBuses(std::vector<std::pair<std::string, std::deque<std::string>>>& buses);

    std::vector<geo::Coordinates> GetAllCoords();

    std::deque<Bus> GetAllBusesSortedByName() const ;

    std::deque<Stop> GetAllStopsSortedByName() const ;

    std::map<std::string, AditionalInfo>& GetAditionalInfo();

    Stop* GetStopByName(const std::string& name);

    bool HaveBuses(const std::string& s);

    const graph::DirectedWeightedGraph<double>& GetGraph();

    const std::unordered_map<std::string_view, size_t>& GetStopNameToVertexId();

    const std::unordered_map<size_t, std::string_view>& GetVertexIdToStopName();

    std::optional<graph::Router<double>::RouteInfo> GetRouteResponse(std::string stop_name_1, std::string stop_name_2);

    const std::set<std::string_view>& GetBusesByStop(const std::string_view& stop_name) const;
    
    const std::deque<Bus>& GetAllBuses() const;
    
    const std::deque<Stop>& GetAllStops() const;
    
    void SetStops(std::deque<Stop>& stops);
    
    void SetBuses(std::deque<Bus>& buses);
    
    void SetStopname_to_stop(std::unordered_map<std::string_view, Stop*>& stopname_to_stop);
    
    void Setbusname_to_bus(std::unordered_map<std::string_view, Bus*>& busname_to_bus);
    
    void SetAditionalinfo(std::map<std::string, AditionalInfo>& aditional_info);
    
    void SetGraph(graph::DirectedWeightedGraph<double>& graph);
    
    const std::unordered_map<std::string_view, Bus*>& GetBusnameToBus() const;
    
    const std::unordered_map<std::string_view, Stop*>& GetStopNameToStop() const;
    
    void SetStopNameToVertexId(std::unordered_map<std::string_view, size_t>& stopname_to_vertex_id, std::unordered_map<size_t, std::string_view> vertex_id_to_stopname);

private:

    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    
    std::unordered_map<std::string_view, std::set<std::string_view>> stopname_to_buses_;
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;
    
    
    std::unordered_map<std::pair<Stop*, Stop*>, double, MyHaser> real_distances_;
    
    
    std::map<std::string, AditionalInfo> aditional_info_;
    
    
    graph::DirectedWeightedGraph<double> graph_;
    graph::TransportRouter transport_router_;
    std::unordered_map<std::string_view, size_t> stopname_to_vertex_id_;
    std::unordered_map<size_t, std::string_view> vertex_id_to_stopname_;
    
};
}