#include "request_handler.h"

RequestHandler::RequestHandler(const TransportCatalogue::TransportCatalogue& db, MapRenderer& renderer) : db_(db), renderer_(renderer) {

}

ResponseForBus RequestHandler::GetBusStat(const std::string& bus_name) const {
    return db_.GetBusResponse(bus_name);
}

const std::set<std::string_view>& RequestHandler::GetBusesByStop(const std::string& stop_name) const {
    return db_.GetBusesByStop(stop_name);
}

svg::Document RequestHandler::RenderMap() {
    return renderer_.Solve();
}
