#include "request_handler.h"

RequestHandler::RequestHandler(const TransportCatalogue& db, const JsonReader& json_reader, const MapRenderer& renderer)
    : db_(db), json_reader_(json_reader), renderer_(renderer) {}

std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    return db_.RequestBus(bus_name);
}

const std::unordered_set<const Bus*>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    return db_.RequestStop(db_.GetStop(stop_name));
}

void RequestHandler::RenderMap(std::ostream& out) const {
    svg::Document document;
    renderer_.GetMapDocument(document, db_);
    document.Render(out);
}
