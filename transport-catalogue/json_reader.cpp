#include "json_reader.h"
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

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
     Solve(os);
}

std::string JsonReader::Print() {
    std::ostringstream os;
    json::Print(main_document_, os);
    return os.str();
}

void JsonReader::Solve(std::ostream& os) {
    if (main_document_.GetRoot().IsMap()) {
        if (main_document_.GetRoot().AsMap().size() == 3) {
            for (auto& [key, arr] : main_document_.GetRoot().AsMap()) {
                if (key == "base_requests"s) {
                    ProcessFill(arr.AsArray());
                } else if (key == "render_settings"s){
                    render_settings_ = ProcessRenderSettings(arr.AsMap());
                } else if (key == "stat_requests"s) {
                    ProcessRequests(arr.AsArray(), os);
                }
            }
        }
    }
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
            if (value.IsMap()) {
                LoadDistances(value.AsMap(), result.second, name);
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
        if (node.IsMap()) {
            const json::Dict& node_as_map = node.AsMap();
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
    std::string name;
    std::vector<std::pair<int, std::pair<std::string, std::string>>> result;
    for (auto& Node : arr) {
        for (auto& [tag, value] : Node.AsMap()) {
            if (tag == "type"s && value.AsString() == "Bus"s) {
                type = "Bus"s;
            } else if (tag == "type"s && value.AsString() == "Stop"s) {
                type = "Stop"s;
            } else if (tag == "type"s && value.AsString() == "Map"s) {
                type = "Map"s;
            } else if (tag == "name"s) {
                name = value.AsString();
            } else if (tag == "id"s) {
                id = value.AsInt();
            }
        }
        result.push_back({id, {type, name}});
    }
    JsonPrinter(result, os);
}

void JsonReader::JsonPrinter(std::vector<std::pair<int, std::pair<std::string, std::string>>>& requests, std::ostream& output) {
    if (!requests.empty()) {
        bool is_first = false;
        output << "["s;
        for (auto& item : requests) {
            if (is_first) {
                output << ", "s;
            }
            output << "{"s;
            output << "\"request_id\": "s << item.first << ", "s;
            if (item.second.first == "Bus"s) {
                Print(transport_catalogue_.GetBusResponse(item.second.second), output);
            } else if (item.second.first == "Stop"s) {
                Print(transport_catalogue_.GetStopResponse(item.second.second), output);
            } else if (item.second.first == "Map"s) {
                output << "\"map\": "s;
                MapRenderer r(render_settings_, transport_catalogue_);
                svg::Document doc = r.Solve();
                r.PrintResult(doc, output);
            } else if (item.second.first == "Unknown"s) {
                output << "\"error_message\": "s << "\"not found\""s;
            }
            output << "}"s;
            is_first = true;
        }
        output << "]"s;
    }
}

void JsonReader::Print(ResponseForBus&& response, std::ostream& output) {
    if (response.is_bus_exist) {
        output << "\"curvature\": "s << response.curvature << ", "s;
        output << "\"route_length\": "s << response.route_real_lenght << ", "s;
        output << "\"stop_count\": "s << response.stops_count << ", "s;
        output << "\"unique_stop_count\": "s << response.unique_stops_count;
    } else {
        output << "\"error_message\": "s << "\"not found\""s;
    }
}

void JsonReader::Print(ResponseForStop&& response, std::ostream& output) {
    if (response.is_stop_exists) {
        output << "\"buses\": ["s;
        if (response.is_buses_exists) {
            bool is_first = false;
            for (auto& bus_name : response.bus_names) {
                if (is_first) {
                    output << ", "s;
                }
                output << "\""s << bus_name << "\""s;
                is_first = true;
            }
        }
        output << "]"s;
    } else {
        output << "\"error_message\": "s << "\"not found\""s;
    }
}
