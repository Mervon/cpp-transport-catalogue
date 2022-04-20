#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const TransportCatalogue::TransportCatalogue& db, MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    ResponseForBus GetBusStat(const std::string& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::set<std::string_view>& GetBusesByStop(const std::string& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap();

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue::TransportCatalogue& db_;
    MapRenderer& renderer_;
};
