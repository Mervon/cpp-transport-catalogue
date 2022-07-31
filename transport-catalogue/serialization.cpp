#include "serialization.h"

#include <iostream>
#include <filesystem>	
#include <fstream>
#include <deque>
#include <algorithm>

using namespace std;

namespace Serialisation {

void Serialise(const TransportCatalogue::TransportCatalogue& tran_cat, const RenderSettings& rend_sett, const RoutingSettings& rout_sett, const std::filesystem::path& path) {
	DataBase data_base;
	
	SerialiseTransportCatalogue(tran_cat, data_base);
	SerialiseRenderSettings(rend_sett, data_base);
	SerialiseRoutingSettings(rout_sett, data_base);
	
	
	ofstream out_file(path, ios::binary);	
	data_base.SerializeToOstream(&out_file);
}

void SerialiseTransportCatalogue(const TransportCatalogue::TransportCatalogue& tran_cat, DataBase& data_base) {
	SerialiseStops(tran_cat, data_base);
	SerialiseBuses(tran_cat, data_base);
	SerialiseAdditionalInfo(const_cast<TransportCatalogue::TransportCatalogue&>(tran_cat), data_base);
	SerialiseGraph(tran_cat, data_base);
	SerialiseStopNameToVertexId(tran_cat, data_base);
}
	
void SerialiseStops(const TransportCatalogue::TransportCatalogue& tc, DataBase& db) {
	const std::deque<::Stop>& stops = tc.GetAllStops();
	
	int counter = 0;
	for (const auto& stop : stops) {
		db.add_stops();
		db.mutable_stops(counter)->set_stop_name(stop.stop_name);
		db.mutable_stops(counter)->mutable_stop_coords()->set_lat(stop.stop_coords.lat);
		db.mutable_stops(counter)->mutable_stop_coords()->set_lng(stop.stop_coords.lng);
		
		++counter;
	}
	
}

void SerialiseBuses(const TransportCatalogue::TransportCatalogue& tc, DataBase& db) {
	const std::deque<::Bus>& buses = tc.GetAllBuses();
	
	int counter = 0;
	for (const auto& bus : buses) {
		db.add_buses();
		db.mutable_buses(counter)->set_bus_name(bus.bus_name);
		db.mutable_buses(counter)->set_unique_stops_count(bus.unique_stops_count);
		db.mutable_buses(counter)->set_route_unreal_lenght(bus.route_unreal_lenght);
		db.mutable_buses(counter)->set_route_real_lenght(bus.route_real_lenght);
		
		svg::Color bus_color = bus.color;
		
		if (holds_alternative<svg::Rgb>(bus_color)) {
			svg::Rgb rgb = get<svg::Rgb>(bus_color);
			db.mutable_buses(counter)->mutable_color()->mutable_rgb_color()->set_red(rgb.red);
			db.mutable_buses(counter)->mutable_color()->mutable_rgb_color()->set_green(rgb.green);
			db.mutable_buses(counter)->mutable_color()->mutable_rgb_color()->set_blue(rgb.blue);
		} else if (holds_alternative<svg::Rgba>(bus_color)) {
			svg::Rgba rgba = get<svg::Rgba>(bus_color);
			db.mutable_buses(counter)->mutable_color()->mutable_rgba_color()->set_red(rgba.red);
			db.mutable_buses(counter)->mutable_color()->mutable_rgba_color()->set_green(rgba.green);
			db.mutable_buses(counter)->mutable_color()->mutable_rgba_color()->set_blue(rgba.blue);
			db.mutable_buses(counter)->mutable_color()->mutable_rgba_color()->set_opacity(rgba.opacity);
		} else if (holds_alternative<std::string>(bus_color)) {
			db.mutable_buses(counter)->mutable_color()->set_string_color(get<string>(bus_color));
		}
	
		for (::Stop* stop : bus.bus_stops) {
			db.mutable_buses(counter)->add_bus_stops(stop->stop_name);
		}

		counter++;
	}
}

void SerialiseAdditionalInfo(TransportCatalogue::TransportCatalogue& tc, DataBase& db) {
	std::map<std::string, ::AditionalInfo>& aditional_info = tc.GetAditionalInfo();
	
	int counter = 0;
	for (auto& [bus_name, last_stop] : aditional_info) {
		db.add_aditional_info();
		db.mutable_aditional_info(counter)->set_bus_name(bus_name);
		db.mutable_aditional_info(counter)->set_last_stop(last_stop.last_stop);
		
		counter++;
	}
	
}

void SerialiseGraph(const TransportCatalogue::TransportCatalogue& tc, DataBase& db) {
	const graph::DirectedWeightedGraph<double>& graph = const_cast<TransportCatalogue::TransportCatalogue&>(tc).GetGraph();
	
	const std::vector<graph::Edge<double>>& edges = graph.GetEdges();
	
	int counter = 0;
	for (const graph::Edge<double>& edge : edges) {
		db.add_graph();
		db.mutable_graph(counter)->set_from(edge.from);
		db.mutable_graph(counter)->set_to_(edge.to);
		db.mutable_graph(counter)->set_weight(edge.weight);
		db.mutable_graph(counter)->set_bus_name(string(edge.bus_name));
		db.mutable_graph(counter)->set_span_count(edge.span_count);
		
		counter++;
	}
}

void SerialiseStopNameToVertexId(const TransportCatalogue::TransportCatalogue& tc, DataBase& db) {
	const std::unordered_map<std::string_view, size_t>& stopname_to_vertex_id = const_cast<TransportCatalogue::TransportCatalogue&>(tc).GetStopNameToVertexId();
	
	int count = 0;
	for (const auto& [stop_name, id] : stopname_to_vertex_id) {
		db.add_stopname_to_vertex_id();
		db.mutable_stopname_to_vertex_id(count)->set_stop_name(string(stop_name));
		db.mutable_stopname_to_vertex_id(count)->set_id(id);
		
		count++;
	}
	
}

void Deserialise(TransportCatalogue::TransportCatalogue& tran_cat, RenderSettings& rend_sett, 
		RoutingSettings& routing_settings, const std::filesystem::path& path) {
	
	ifstream in_file(path, ios::binary);
	
	DataBase data_base;
	
	data_base.ParseFromIstream(&in_file);
	
	DeserialiseTransportCatalogue(tran_cat, data_base);
	
	DeserialiseRenderSettings(rend_sett, data_base);
	
	DeserialiseRoutingSettings(routing_settings, data_base);
}

void DeserialiseTransportCatalogue(TransportCatalogue::TransportCatalogue& tc, DataBase& data_base) {

	int size_of_stops = data_base.stops_size();
		
	std::deque<::Stop> stops;
	
	for (int i = 0; i < size_of_stops; ++i) {
		
		string stop_name = std::move(*(data_base.mutable_stops(i)->mutable_stop_name()));
		
		geo::Coordinates coordinates{std::move(data_base.mutable_stops(i)->mutable_stop_coords()->lat()),
								 	 std::move(data_base.mutable_stops(i)->mutable_stop_coords()->lng())};
		
		stops.push_back(::Stop{std::move(stop_name), std::move(coordinates)});
		
	}
	
	graph::DirectedWeightedGraph<double> graph(stops.size());
	
	tc.SetStops(stops);
	
	const std::deque<::Stop>& stops_ = tc.GetAllStops();
	
	int size_of_buses = data_base.buses_size();
		
	std::deque<::Bus> buses;
	
	for (int i = 0; i < size_of_buses; ++i) {

		string bus_name = std::move(*(data_base.mutable_buses(i)->mutable_bus_name()));
		
		int unique_stops_count = data_base.buses(i).unique_stops_count();
		double route_unreal_lenght = data_base.buses(i).route_unreal_lenght();
		double route_real_lenght = data_base.buses(i).route_real_lenght();

		svg::Color color;
		
		if (data_base.mutable_buses(i)->mutable_color()->has_rgb_color()) {
			svg::Rgb rgb;
			rgb.red = data_base.mutable_buses(i)->mutable_color()->mutable_rgb_color()->red();
			rgb.green = data_base.mutable_buses(i)->mutable_color()->mutable_rgb_color()->green();
			rgb.blue = data_base.mutable_buses(i)->mutable_color()->mutable_rgb_color()->blue();
			color = rgb;
		} else if (data_base.mutable_buses(i)->mutable_color()->has_rgba_color()) {
			svg::Rgba rgba;
			rgba.red = data_base.mutable_buses(i)->mutable_color()->mutable_rgba_color()->red();
			rgba.green = data_base.mutable_buses(i)->mutable_color()->mutable_rgba_color()->green();
			rgba.blue = data_base.mutable_buses(i)->mutable_color()->mutable_rgba_color()->blue();
			rgba.opacity = data_base.mutable_buses(i)->mutable_color()->mutable_rgba_color()->opacity();
			color = rgba;
		} else {
			color = *(data_base.mutable_buses(i)->mutable_color()->mutable_string_color());
		}
			
		std::vector<::Stop*> bus_stops;
		
		int size_of_stops = data_base.mutable_buses(i)->bus_stops_size();
		
		for (int j = 0; j < size_of_stops; ++j) {
			string stop_name_to_find = std::move(data_base.buses(i).bus_stops(j));
			
			auto it = std::find_if(stops_.begin(), stops_.end(), [&stop_name_to_find](const ::Stop& stop) {
				return stop.stop_name == stop_name_to_find;
			});
			
			bus_stops.push_back(const_cast<::Stop*>(&(*it)));
		}
		
		buses.push_back(::Bus{std::move(color), std::move(bus_name), std::move(bus_stops), unique_stops_count, route_unreal_lenght, route_real_lenght}); 
	}
	
	tc.SetBuses(buses);
	
	const std::deque<::Bus>& buses_ = tc.GetAllBuses();
	
	std::unordered_map<std::string_view, ::Stop*> stopname_to_stop;
	
	DeserialiseStopNameToStop(stopname_to_stop, stops_);
	
	tc.SetStopname_to_stop(stopname_to_stop);
	
	std::unordered_map<std::string_view, ::Bus*> busname_to_bus;
	
	DeserialiseBusNameToBus(busname_to_bus, buses_);
	
	tc.Setbusname_to_bus(busname_to_bus);

	tc.InitStopnameToBuses();		
	
	std::map<std::string, ::AditionalInfo> aditional_info;
	
	DeserialiseAditionalInfo(aditional_info, data_base);
	
	tc.SetAditionalinfo(aditional_info);
	
	DeserialiseGraph(tc, graph, data_base);
	
	tc.SetGraph(graph);

	std::unordered_map<std::string_view, size_t> stopname_to_vertex_id;
	std::unordered_map<size_t, std::string_view> vertex_id_to_stopname;
	
	DeserialiseStopNameToVertexId(tc, stopname_to_vertex_id, vertex_id_to_stopname, data_base);
	
	tc.SetStopNameToVertexId(stopname_to_vertex_id, vertex_id_to_stopname);
}

void DeserialiseStopNameToStop(std::unordered_map<std::string_view, ::Stop*>& stopname_to_stop, const std::deque<::Stop>& stops) {
	for (const ::Stop& stop : stops) {
		stopname_to_stop[stop.stop_name] = const_cast<::Stop*>(&stop);
	}
}

void DeserialiseBusNameToBus(std::unordered_map<std::string_view, ::Bus*>& busname_to_bus, const std::deque<::Bus>& buses) {
	for (const ::Bus& bus : buses) {
		busname_to_bus[bus.bus_name] = const_cast<::Bus*>(&bus);
	}
}

void DeserialiseAditionalInfo(std::map<std::string, ::AditionalInfo>& aditional_info, DataBase& data_base) {
	int add_info_size = data_base.aditional_info_size();
	
	for (int i = 0; i < add_info_size; ++i) {
		aditional_info[data_base.mutable_aditional_info(i)->bus_name()].last_stop = *(data_base.mutable_aditional_info(i)->mutable_last_stop());
	}
	
}

void DeserialiseRenderSettings(RenderSettings& rend_sett, DataBase& data_base) {
	rend_sett.width = data_base.render_sett().width();
	rend_sett.height = data_base.render_sett().height();
	rend_sett.padding = data_base.render_sett().padding();
	rend_sett.line_width = data_base.render_sett().line_width();
	rend_sett.stop_radius = data_base.render_sett().stop_radius();
	rend_sett.bus_label_font_size = data_base.render_sett().bus_label_font_size();
	rend_sett.bus_label_offset.first = data_base.render_sett().bus_label_offset_1();
	rend_sett.bus_label_offset.second = data_base.render_sett().bus_label_offset_2();
	rend_sett.stop_label_font_size = data_base.render_sett().stop_label_font_size();
    rend_sett.stop_label_offset.first = data_base.render_sett().stop_label_offset_1();
    rend_sett.stop_label_offset.second = data_base.render_sett().stop_label_offset_2();
    rend_sett.underlayer_width = data_base.render_sett().underlayer_width();
    
    int size_of_color_pallete = data_base.render_sett().color_palette_size();
    std::vector<svg::Color> color_palette;
    for (int i = 0; i < size_of_color_pallete; ++i) {
    	if (data_base.mutable_render_sett()->color_palette(i).has_rgb_color()) {
    		svg::Rgb rgb;
    		rgb.red = data_base.mutable_render_sett()->mutable_color_palette(i)->mutable_rgb_color()->red();
    		rgb.green = data_base.mutable_render_sett()->mutable_color_palette(i)->mutable_rgb_color()->green();
    		rgb.blue = data_base.mutable_render_sett()->mutable_color_palette(i)->mutable_rgb_color()->blue();
    		color_palette.push_back(svg::Color{rgb});
    	} else if (data_base.mutable_render_sett()->color_palette(i).has_rgba_color()) {
    		svg::Rgba rgba;
			rgba.red = data_base.mutable_render_sett()->mutable_color_palette(i)->mutable_rgba_color()->red();
			rgba.green = data_base.mutable_render_sett()->mutable_color_palette(i)->mutable_rgba_color()->green();
			rgba.blue = data_base.mutable_render_sett()->mutable_color_palette(i)->mutable_rgba_color()->blue();
			rgba.opacity = data_base.mutable_render_sett()->mutable_color_palette(i)->mutable_rgba_color()->opacity();
			color_palette.push_back(svg::Color{rgba});
    	} else {
    		color_palette.push_back(svg::Color{data_base.mutable_render_sett()->mutable_color_palette(i)->string_color()});
    	}
    }
    rend_sett.color_palette = std::move(color_palette);
    
    if (data_base.mutable_render_sett()->underlayer_color().has_rgb_color()) {
		svg::Rgb rgb;
		rgb.red = data_base.mutable_render_sett()->mutable_underlayer_color()->mutable_rgb_color()->red();
		rgb.green = data_base.mutable_render_sett()->mutable_underlayer_color()->mutable_rgb_color()->green();
		rgb.blue = data_base.mutable_render_sett()->mutable_underlayer_color()->mutable_rgb_color()->blue();
		rend_sett.underlayer_color = svg::Color{rgb};
	} else if (data_base.mutable_render_sett()->underlayer_color().has_rgba_color()) {
		svg::Rgba rgba;
		rgba.red = data_base.mutable_render_sett()->mutable_underlayer_color()->mutable_rgba_color()->red();
		rgba.green = data_base.mutable_render_sett()->mutable_underlayer_color()->mutable_rgba_color()->green();
		rgba.blue = data_base.mutable_render_sett()->mutable_underlayer_color()->mutable_rgba_color()->blue();
		rgba.opacity = data_base.mutable_render_sett()->mutable_underlayer_color()->mutable_rgba_color()->opacity();
		rend_sett.underlayer_color = svg::Color{rgba};
	} else {
		rend_sett.underlayer_color = svg::Color{data_base.mutable_render_sett()->mutable_underlayer_color()->string_color()};
	}
}

void DeserialiseRoutingSettings(RoutingSettings& routing_settings, DataBase& data_base) {
	routing_settings.bus_wait_time_ = data_base.routing_sett().bus_wait_time();
	routing_settings.bus_velocity = data_base.routing_sett().bus_velocity();
}

void DeserialiseGraph(TransportCatalogue::TransportCatalogue& tc, graph::DirectedWeightedGraph<double>& graph, DataBase& data_base) {
	int size_of_graph = data_base.graph_size();
	
	const std::unordered_map<std::string_view, ::Bus*>& busname_to_bus = const_cast<TransportCatalogue::TransportCatalogue&>(tc).GetBusnameToBus();
	
	for (int i = 0; i < size_of_graph; ++i) {
		graph::Edge<double> edge;
		edge.from = data_base.mutable_graph(i)->from();
		edge.to = data_base.mutable_graph(i)->to_();
		edge.weight = data_base.mutable_graph(i)->weight();
		auto it = busname_to_bus.find(data_base.mutable_graph(i)->bus_name());
		edge.bus_name = it->first;
		
		edge.span_count = data_base.mutable_graph(i)->span_count();
	
		graph.AddEdge(edge);
	}
}

void DeserialiseStopNameToVertexId(TransportCatalogue::TransportCatalogue& tc, std::unordered_map<std::string_view,
		size_t>& stopname_to_vertex_id, std::unordered_map<size_t, std::string_view>& vertex_id_to_stopname, DataBase& data_base) {
	
	int size_of_stopname_to_vertex_id = data_base.stopname_to_vertex_id_size();
	
	const std::unordered_map<std::string_view, ::Stop*>& stopname_to_stop = const_cast<TransportCatalogue::TransportCatalogue&>(tc).GetStopNameToStop();
	
	for (int i = 0; i < size_of_stopname_to_vertex_id; ++i) {
		auto it = stopname_to_stop.find(data_base.mutable_stopname_to_vertex_id(i)->stop_name());
		string_view stop_name = it->first;
		stopname_to_vertex_id[stop_name] = data_base.mutable_stopname_to_vertex_id(i)->id();
		vertex_id_to_stopname[data_base.mutable_stopname_to_vertex_id(i)->id()] = stop_name;
	}
}

void SerialiseRenderSettings(const RenderSettings& rend_sett, DataBase& data_base) {
	data_base.mutable_render_sett()->set_width(rend_sett.width);
	data_base.mutable_render_sett()->set_height(rend_sett.height);
	data_base.mutable_render_sett()->set_padding(rend_sett.padding);
	data_base.mutable_render_sett()->set_line_width(rend_sett.line_width);
	data_base.mutable_render_sett()->set_stop_radius(rend_sett.stop_radius);
	data_base.mutable_render_sett()->set_bus_label_offset_1(rend_sett.bus_label_offset.first);
	data_base.mutable_render_sett()->set_bus_label_offset_2(rend_sett.bus_label_offset.second);
	data_base.mutable_render_sett()->set_stop_label_offset_1(rend_sett.stop_label_offset.first);
	data_base.mutable_render_sett()->set_stop_label_offset_2(rend_sett.stop_label_offset.second);
	data_base.mutable_render_sett()->set_bus_label_font_size(rend_sett.bus_label_font_size);
	data_base.mutable_render_sett()->set_stop_label_font_size(rend_sett.stop_label_font_size);
	data_base.mutable_render_sett()->set_underlayer_width(rend_sett.underlayer_width);
	
	SerialiseUnderlayerColor(rend_sett.underlayer_color, data_base);
	
	SerialiseColorPalette(rend_sett.color_palette, data_base);
}

void SerialiseRoutingSettings(const RoutingSettings& routing_settings, DataBase& data_base) {
	data_base.mutable_routing_sett()->set_bus_wait_time(routing_settings.bus_wait_time_);
	data_base.mutable_routing_sett()->set_bus_velocity(routing_settings.bus_velocity);
}

void SerialiseUnderlayerColor(const svg::Color& color, DataBase& data_base) {	
	if (holds_alternative<svg::Rgb>(color)) {
		svg::Rgb rgb = get<svg::Rgb>(color);
		data_base.mutable_render_sett()->mutable_underlayer_color()->mutable_rgb_color()->set_red(rgb.red);
		data_base.mutable_render_sett()->mutable_underlayer_color()->mutable_rgb_color()->set_green(rgb.green);
		data_base.mutable_render_sett()->mutable_underlayer_color()->mutable_rgb_color()->set_blue(rgb.blue);
	} else if (holds_alternative<svg::Rgba>(color)) {
		svg::Rgba rgba = get<svg::Rgba>(color);
		data_base.mutable_render_sett()->mutable_underlayer_color()->mutable_rgba_color()->set_red(rgba.red);
		data_base.mutable_render_sett()->mutable_underlayer_color()->mutable_rgba_color()->set_green(rgba.green);
		data_base.mutable_render_sett()->mutable_underlayer_color()->mutable_rgba_color()->set_blue(rgba.blue);
		data_base.mutable_render_sett()->mutable_underlayer_color()->mutable_rgba_color()->set_opacity(rgba.opacity);
	} else if (holds_alternative<std::string>(color)) {
		data_base.mutable_render_sett()->mutable_underlayer_color()->set_string_color(get<string>(color));
	}
}

void SerialiseColorPalette(const std::vector<svg::Color>& color_palette, DataBase& data_base) {
	int counter = 0;
	
	for (const svg::Color& color : color_palette) {
		data_base.mutable_render_sett()->add_color_palette();

		if (holds_alternative<svg::Rgb>(color)) {
			svg::Rgb rgb = get<svg::Rgb>(color);
			data_base.mutable_render_sett()->mutable_color_palette(counter)->mutable_rgb_color()->set_red(rgb.red);
			data_base.mutable_render_sett()->mutable_color_palette(counter)->mutable_rgb_color()->set_green(rgb.green);
			data_base.mutable_render_sett()->mutable_color_palette(counter)->mutable_rgb_color()->set_blue(rgb.blue);
		} else if (holds_alternative<svg::Rgba>(color)) {
			svg::Rgba rgba = get<svg::Rgba>(color);
			data_base.mutable_render_sett()->mutable_color_palette(counter)->mutable_rgba_color()->set_red(rgba.red);
			data_base.mutable_render_sett()->mutable_color_palette(counter)->mutable_rgba_color()->set_green(rgba.green);
			data_base.mutable_render_sett()->mutable_color_palette(counter)->mutable_rgba_color()->set_blue(rgba.blue);
			data_base.mutable_render_sett()->mutable_color_palette(counter)->mutable_rgba_color()->set_opacity(rgba.opacity);
		} else if (holds_alternative<std::string>(color)) {
			data_base.mutable_render_sett()->mutable_color_palette(counter)->set_string_color(get<string>(color));
		}
		
		counter++;
	}
}

}
