#pragma once

#include <filesystem>

#include "json.h"
#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"
#include "map_renderer.h"
#include "json_builder.h"

namespace Serialisation {

void Serialise(const TransportCatalogue::TransportCatalogue& tran_cat, const RenderSettings& rend_sett, const RoutingSettings& routing_settings, const std::filesystem::path& path);


void SerialiseTransportCatalogue(const TransportCatalogue::TransportCatalogue& tran_cat, DataBase& data_base);
void SerialiseStops(const TransportCatalogue::TransportCatalogue& tc, DataBase& db);
void SerialiseBuses(const TransportCatalogue::TransportCatalogue& tc, DataBase& db);
void SerialiseAdditionalInfo(TransportCatalogue::TransportCatalogue& tc, DataBase& db);
void SerialiseGraph(const TransportCatalogue::TransportCatalogue& tc, DataBase& db);
void SerialiseStopNameToVertexId(const TransportCatalogue::TransportCatalogue& tc, DataBase& db);

void SerialiseRenderSettings(const RenderSettings& rend_sett, DataBase& data_base);
void SerialiseUnderlayerColor(const svg::Color& color, DataBase& data_base);
void SerialiseColorPalette(const std::vector<svg::Color>& color_palette, DataBase& data_base);

void SerialiseRoutingSettings(const RoutingSettings& routing_settings, DataBase& data_base);


void Deserialise(TransportCatalogue::TransportCatalogue& tran_cat, RenderSettings& rend_sett, RoutingSettings& routing_settings, const std::filesystem::path& path);


void DeserialiseTransportCatalogue(TransportCatalogue::TransportCatalogue& tran_cat, DataBase& data_base);
void DeserialiseStopNameToStop(std::unordered_map<std::string_view, ::Stop*>& stopname_to_stop, const std::deque<::Stop>& stops);
void DeserialiseBusNameToBus(std::unordered_map<std::string_view, ::Bus*>& busname_to_bus, const std::deque<::Bus>& buses);
void DeserialiseAditionalInfo(std::map<std::string, ::AditionalInfo>& aditional_info, DataBase& data_base);
void DeserialiseGraph(TransportCatalogue::TransportCatalogue& tc, graph::DirectedWeightedGraph<double>& graph, DataBase& data_base);
void DeserialiseStopNameToVertexId(TransportCatalogue::TransportCatalogue& tc, std::unordered_map<std::string_view,
		size_t>& stopname_to_vertex_id, std::unordered_map<size_t, std::string_view>& vertex_id_to_stopname, DataBase& data_base);

void DeserialiseRenderSettings(RenderSettings& rend_sett, DataBase& data_base);

void DeserialiseRoutingSettings(RoutingSettings& routing_settings, DataBase& data_base);

}