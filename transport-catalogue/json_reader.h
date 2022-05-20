#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_builder.h"

#include <utility>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

class JsonReader {
public:
    JsonReader() = default;

    JsonReader(json::Document d);

    JsonReader(std::string s);

    JsonReader(std::istream& is, std::ostream& os);

    std::string Print();

private:
    json::Document main_document_;
    RenderSettings render_settings_;
    RoutingSettings routing_settings_;
    TransportCatalogue::TransportCatalogue transport_catalogue_;

    void Solve(std::ostream& os);

    void FillStop(const json::Dict& dict, std::string& name, std::pair<std::vector<Stop>, std::map<std::pair<std::string, std::string>, double>>& result);

    void LoadDistances(const json::Dict& dict, std::map<std::pair<std::string, std::string>, double>& result, std::string& main_stop_name);

    void FillBus(const json::Dict& dict, std::string& name, std::vector<std::pair<std::string, std::deque<std::string>>>& buses, std::map<std::string, AditionalInfo>& aditional_info);

    void LoadBusStops(std::vector<std::pair<std::string, std::deque<std::string>>>& result, const json::Array& arr);

    void ProcessFill(const json::Array& arr);

    void LoadJSON(std::string s);

    RoutingSettings& ProcessRoutingSettings(const json::Dict& dict);

    std::pair<std::string, std::string> CheckTypeAndName(const json::Dict& dict);

    RenderSettings ProcessRenderSettings(const json::Dict& dict);

    void ProcessRequests(const json::Array& arr, std::ostream& os);

    void JsonPrinter(std::vector<std::pair<int, std::pair<std::string, RequestInfo>>>& requests, std::ostream& output);

    void Print(ResponseForBus&& response, json::Builder& builder);

    void Print(ResponseForStop&& response, json::Builder& builder);

    void Print(std::optional<graph::Router<double>::RouteInfo>&& route_info, json::Builder& builder);
};
