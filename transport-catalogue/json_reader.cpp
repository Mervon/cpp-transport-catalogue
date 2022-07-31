#include "json_reader.h"
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "serialization.h"

#include <fstream>
#include <filesystem>
#include <cassert>
#include <utility>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>


using namespace std;

JsonReader::JsonReader(json::Document d) : main_document_(std::move(d)) {

}

JsonReader::JsonReader(std::string s) {
    LoadJSON(s);
}

JsonReader::JsonReader(std::istream& is, std::ostream& os) {
    main_document_ = json::Load(is);
}

JsonReader::JsonReader(const WorkModes& work_mode) {
	switch(work_mode) {
		case WorkModes::MakeBase: {
			
			main_document_ = json::Load(std::cin);
			
			path_ = main_document_.GetRoot().AsDict().at("serialization_settings"s).AsDict().at("file"s).AsString();
			
			MakeBase();
			
			Serialisation::Serialise(transport_catalogue_, render_settings_, routing_settings_, path_);
			
			break;
		}
		case WorkModes::ProcessRequests: {
			
			main_document_ = json::Load(std::cin);
			
			path_ = main_document_.GetRoot().AsDict().at("serialization_settings"s).AsDict().at("file"s).AsString();
			
			Serialisation::Deserialise(transport_catalogue_, render_settings_, routing_settings_, path_);
			
			ProcessRequests(main_document_.GetRoot().AsDict().at("stat_requests"s).AsArray(), std::cout);
			
			break;
		}
		default: break;
	}

}

std::string JsonReader::Print() {
    std::ostringstream os;
    json::Print(main_document_, os);
    return os.str();
}

void JsonReader::MakeBase() {
	
	ProcessFill(main_document_.GetRoot().AsDict().at("base_requests"s).AsArray());
	
	render_settings_ = ProcessRenderSettings(main_document_.GetRoot().AsDict().at("render_settings"s).AsDict());
	
	transport_catalogue_.ProcessGraph(ProcessRoutingSettings(main_document_.GetRoot().AsDict().at("routing_settings"s).AsDict()));
	
}

RoutingSettings& JsonReader::ProcessRoutingSettings(const json::Dict& dict) {
    RoutingSettings settings;
    for (auto& [key, value] : dict) {
        if (key == "bus_wait_time"sv) {
            settings.bus_wait_time_ = value.AsInt();
        } else if (key == "bus_velocity"sv) {
            settings.bus_velocity = value.AsDouble();
        }
    }
    routing_settings_ = move(settings);
    return routing_settings_;
}

svg::Color GetColor(const json::Node& clr) {
    if (clr.IsString()) {
        return clr.AsString();
    } else if (clr.IsArray()) {
        if (clr.AsArray().size() == 3) {
            svg::Rgb rgb;
            rgb.red = clr.AsArray()[0].AsInt();
            rgb.green = clr.AsArray()[1].AsInt();
            rgb.blue = clr.AsArray()[2].AsInt();
            return rgb;
        } else if (clr.AsArray().size() == 4) {
            svg::Rgba rgba;
            rgba.red = clr.AsArray()[0].AsInt();
            rgba.green = clr.AsArray()[1].AsInt();
            rgba.blue = clr.AsArray()[2].AsInt();
            rgba.opacity = clr.AsArray()[3].AsDouble();
            return rgba;
        } else {
            return {};
        }
    } else {
        return {};
    }
}

vector<svg::Color> GetColors(const json::Array& arr) {
    vector<svg::Color> result;
    for (auto& clr : arr) {
        result.push_back(GetColor(clr));
    }
    return result;
}

RenderSettings JsonReader::ProcessRenderSettings(const json::Dict& dict) {
    RenderSettings render_settings;
    for (auto& [tag, value] : dict) {
        if (tag == "width"s) {
            render_settings.width = value.AsDouble();
        } else if (tag == "height"s) {
            render_settings.height = value.AsDouble();
        } else if (tag == "padding"s) {
            render_settings.padding = value.AsDouble();
        } else if (tag == "line_width"s) {
            render_settings.line_width = value.AsDouble();
        } else if (tag == "stop_radius"s) {
            render_settings.stop_radius = value.AsDouble();
        } else if (tag == "bus_label_font_size"s) {
            render_settings.bus_label_font_size = value.AsInt();
        } else if (tag == "bus_label_offset"s) {
            render_settings.bus_label_offset = {value.AsArray()[0].AsDouble(), value.AsArray()[1].AsDouble()};
        } else if (tag == "stop_label_font_size"s) {
            render_settings.stop_label_font_size = value.AsInt();
        } else if (tag == "stop_label_offset"s) {
            render_settings.stop_label_offset = {value.AsArray()[0].AsDouble(), value.AsArray()[1].AsDouble()};
        } else if (tag == "underlayer_color"s) {
            render_settings.underlayer_color = GetColor(value);
        } else if (tag == "underlayer_width"s) {
            render_settings.underlayer_width = value.AsDouble();
        } else if (tag == "color_palette"s) {
            render_settings.color_palette = GetColors(value.AsArray());
        }
    }
    return render_settings;
}

void JsonReader::FillStop(const json::Dict& dict, std::string& name, std::pair<std::vector<Stop>, std::map<std::pair<std::string, std::string>, double>>& result) {

    Stop stop;
    stop.stop_name = name;
    for (auto& [tag, value] : dict) {
        if (tag == "latitude"s) {
            stop.stop_coords.lat = value.AsDouble();
        } else if (tag == "longitude"s) {
            stop.stop_coords.lng = value.AsDouble();
        } else if (tag == "road_distances"s) {
            if (value.IsDict()) {
                LoadDistances(value.AsDict(), result.second, name);
            }
        }
    }
    result.first.push_back(stop);
}

void JsonReader::LoadDistances(const json::Dict& dict, std::map<std::pair<std::string, std::string>, double>& result, std::string& main_stop_name) {
    for (auto& [stop_name, distance] : dict) {
        result[{main_stop_name, stop_name}] = distance.AsDouble();
    }
}

void JsonReader::FillBus(const json::Dict& dict, std::string& name, std::vector<std::pair<std::string, std::deque<std::string>>>& buses, map<string, AditionalInfo>& aditional_info) {
    buses.push_back({name, {}});
    bool is_round = false;
    for (auto& [tag, value] : dict) {
        if (tag == "stops"s) {
            if (value.IsArray()) {
                LoadBusStops(buses, value.AsArray());
            }
        } else if (tag == "is_roundtrip"s) {
            if (value.AsBool()) {
                is_round = true;
            }
        }
    }

    if (!is_round) {
        AditionalInfo ai;
        if (*(buses.back().second.begin()) != *(--(buses.back().second.end()))) {
            AditionalInfo ai;
            ai.last_stop = *(--(buses.back().second.end()));
            aditional_info[name] = ai;
        }
        if (buses.back().second.size() > 1) {
            std::deque<std::string>::iterator it_to_last_elem = --(--buses.back().second.end());
            std::deque<std::string>::iterator it_to_first_elem = buses.back().second.begin();
            for (;it_to_last_elem != it_to_first_elem; it_to_last_elem--) {
                buses.back().second.push_back(*it_to_last_elem);
            }
            buses.back().second.push_back(*it_to_last_elem);
        }
    }
}

void JsonReader::LoadBusStops(std::vector<std::pair<std::string, std::deque<std::string>>>& result, const json::Array& arr) {
    for (auto& stop_name : arr) {
        result.back().second.push_back(stop_name.AsString());
    }
}

void JsonReader::ProcessFill(const json::Array& arr) {

    std::pair<std::vector<Stop>, std::map<std::pair<std::string, std::string>, double>> res1;
    std::vector<std::pair<std::string, std::deque<std::string>>> res2;
    std::map<std::string, AditionalInfo> aditional_info;

    for (auto& node : arr) {
        if (node.IsDict()) {
            const json::Dict& node_as_map = node.AsDict();
            std::pair<std::string, std::string> type_and_name = CheckTypeAndName(node_as_map);

            if (type_and_name.first == "Stop"s) {
                FillStop(node_as_map, type_and_name.second, res1);
            } else if (type_and_name.first == "Bus"s) {
                FillBus(node_as_map, type_and_name.second, res2, aditional_info);
            }
        }
    }

    TransportCatalogue::TransportCatalogue transport_catalogue(res1.first, res2, res1.second, aditional_info);

    transport_catalogue_ = std::move(transport_catalogue);

}

void JsonReader::LoadJSON(std::string s) {
    std::istringstream is(s);
    main_document_ = json::Load(is);
}

std::pair<std::string, std::string> JsonReader::CheckTypeAndName(const json::Dict& dict) {
    std::pair<std::string, std::string> result;
    for (auto& [tag, value] : dict) {
        if (tag == "type"s && value.AsString() == "Stop"s) {
            result.first = "Stop"s;
        } else if (tag == "type"s && value.AsString() == "Bus"s) {
            result.first = "Bus"s;
        } else if (tag == "name"s) {
            result.second = value.AsString();
        }
    }
    return result;
}

void JsonReader::ProcessRequests(const json::Array& arr, std::ostream& os) {
	
    int id;
    std::string type = "Unknown"s;

    RequestInfo request_info;
    std::vector<std::pair<int, std::pair<std::string, RequestInfo>>> result;
    for (auto& Node : arr) {
        for (auto& [tag, value] : Node.AsDict()) {
            if (tag == "type"s && value.AsString() == "Bus"s) {
                type = "Bus"s;
            } else if (tag == "type"s && value.AsString() == "Stop"s) {
                type = "Stop"s;
            } else if (tag == "type"s && value.AsString() == "Map"s) {
                type = "Map"s;
            } else if (tag == "type"s && value.AsString() == "Route"s) {
                type = "Route"s;
            } else if (tag == "name"s) {
                request_info.name = value.AsString();
            } else if (tag == "id"s) {
                id = value.AsInt();
            } else if (tag == "from"s) {
                request_info.from = value.AsString();
            } else if (tag == "to"s) {
                request_info.to = value.AsString();
            }
        }
        result.push_back({id, {type, request_info}});
    }

    JsonPrinter(result, os);

}

void JsonReader::JsonPrinter(std::vector<std::pair<int, std::pair<std::string, RequestInfo>>>& requests, std::ostream& output) {
	
    if (!requests.empty()) {
        json::Builder builder;
        builder.StartArray();
        
        for (auto& item : requests) {
            builder.StartDict();
            builder.Key("request_id"s).Value(item.first);
            if (item.second.first == "Bus"s) {
                Print(transport_catalogue_.GetBusResponse(item.second.second.name), builder);
            } else if (item.second.first == "Stop"s) {
                Print(transport_catalogue_.GetStopResponse(item.second.second.name), builder);
            } else if (item.second.first == "Map"s) {
                MapRenderer r(render_settings_, transport_catalogue_);
                svg::Document doc = r.Solve();
                builder.Key("map"s).Value(r.PrintResult(doc));
            } else if (item.second.first == "Route"s) {
            	
                Print(transport_catalogue_.GetRouteResponse(item.second.second.from, item.second.second.to), builder);
                
            } else if (item.second.first == "Unknown"s) {
                builder.Key("error_message"s).Value(json::Node{"not found"s});
            }
            builder.EndDict();
        }

        builder.EndArray();

        json::Print(json::Document{builder.Build()}, output);

    }
    
}

void JsonReader::Print(ResponseForBus&& response, json::Builder& builder) {
    if (response.is_bus_exist) {
        builder.Key("curvature"s).Value(response.curvature);
        builder.Key("route_length"s).Value(response.route_real_lenght);
        builder.Key("stop_count"s).Value(static_cast<int>(response.stops_count));
        builder.Key("unique_stop_count"s).Value(static_cast<int>(response.unique_stops_count));
    } else {
        builder.Key("error_message"s).Value(json::Node{"not found"s});
    }
}

void JsonReader::Print(ResponseForStop&& response, json::Builder& builder) {
    if (response.is_stop_exists) {
        builder.Key("buses"s).StartArray();
        if (response.is_buses_exists) {
            for (auto& bus_name : response.bus_names) {
                builder.Value(static_cast<string>(bus_name));
            }
        }
        builder.EndArray();
    } else {
        builder.Key("error_message"s).Value(json::Node{"not found"s});
    }
}

void JsonReader::Print(std::optional<graph::Router<double>::RouteInfo>&& route_info, json::Builder& builder) {
	
    if (route_info) {
        builder.Key("total_time"s).Value((*route_info).weight);

        builder.Key("items"s).StartArray();
        std::vector<graph::EdgeId>& edges = (*route_info).edges;
        const graph::DirectedWeightedGraph<double>& graph = transport_catalogue_.GetGraph();
        const std::unordered_map<size_t, std::string_view>& vertex_id_to_stopname = transport_catalogue_.GetVertexIdToStopName();
        
        for (const auto& EdgeId : edges) {
        	
            graph::Edge<double> edge = graph.GetEdge(EdgeId);
            graph::VertexId from = edge.from;
            double weight = edge.weight;
            int span_count = edge.span_count;
            
            builder.StartDict();
           
                builder.Key("stop_name"s).Value(string(vertex_id_to_stopname.at(from)));
                
                builder.Key("time"s).Value(routing_settings_.bus_wait_time_);
                builder.Key("type"s).Value("Wait"s);
            builder.EndDict();
            builder.StartDict();
                builder.Key("bus"s).Value(string(edge.bus_name));
                builder.Key("span_count"s).Value(span_count);
                builder.Key("time"s).Value(static_cast<double>(weight - routing_settings_.bus_wait_time_));
                builder.Key("type"s).Value("Bus"s);
            builder.EndDict();
            

        }
        builder.EndArray();
    } else {

        builder.Key("error_message"s).Value(json::Node{"not found"s});

    }
   

}