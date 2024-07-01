#include "request_handler.h"

RequestHandler::RequestHandler(const TransportCatalogue& db, const JsonReader& json_reader, RenderSettings settings)
    : db_(db), json_reader_(json_reader), renderer_(settings) {}

std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    return db_.RequestBus(bus_name);
}

const std::unordered_set<const Bus*>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    return db_.RequestStop(db_.GetStop(stop_name));
}

Document RequestHandler::GetRequestDocument() const {
    using namespace std::literals;

    std::vector<Node> res;
    for (const Node& request_node : json_reader_.GetRequestArray()) {
        Dict request_map = request_node.AsMap();
        if (request_map.at("type"s).AsString() == "Bus"s) {
            BusStat stat = db_.RequestBus(request_map.at("name"s).AsString());
            Dict r;
            MakeBusDict(r, stat, request_map);
            res.emplace_back(move(r));
        }
        else if (request_map.at("type"s).AsString() == "Stop"s) {
            Dict r;
            MakeStopDict(r, request_map);
            res.emplace_back(move(r));
        }
        else if (request_map.at("type"s).AsString() == "Map"s) {
            Dict r;
            MakeMapDict(r, request_map.at("id"s));
            res.emplace_back(move(r));
        }
    }
    return Document{ Node{res} };

}

void RequestHandler::MakeBusDict(json::Dict& r, BusStat stat, const json::Dict& request_map) const {
    using namespace std::literals;

    if (stat.total_stops == 0) {
        r.emplace("request_id"s, Node{ request_map.at("id"s) });
        r.emplace("error_message"s, Node{ "not found"s });
        return;
    }
    r.emplace("curvature"s, Node{ stat.curvature });
    r.emplace("request_id"s, Node{ request_map.at("id"s) });
    r.emplace("route_length"s, Node{ stat.route_length });
    r.emplace("stop_count"s, Node{ static_cast<int>(stat.total_stops) });
    r.emplace("unique_stop_count"s, Node{ static_cast<int>(stat.unique_stops) });
    return;
}

void RequestHandler::MakeStopDict(json::Dict& r, const json::Dict& request_map) const {
    using namespace std::literals;

    if (db_.GetStop(request_map.at("name"s).AsString()) == nullptr) {
        r.emplace("request_id"s, Node{ request_map.at("id"s) });
        r.emplace("error_message"s, Node{ "not found"s });
        return;
    }
    if (db_.RequestStop(db_.GetStop(request_map.at("name"s).AsString())) == nullptr) {
        r.emplace("buses"s, std::vector<Node>{});
        r.emplace("request_id"s, Node{ request_map.at("id"s) });
        return;
    }
    std::vector<std::string> vec;
    for (BusPtr bus : *db_.RequestStop(db_.GetStop(request_map.at("name"s).AsString()))) {
        vec.emplace_back(bus->bus_name);
    }
    sort(vec.begin(), vec.end(), [](const std::string& lhs, const std::string& rhs) {
        return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
        });
    std::vector<Node> bus_names;
    for (const std::string& bus_name : vec) {
        bus_names.emplace_back(Node{ bus_name });
    }
    r.emplace("buses"s, bus_names);
    r.emplace("request_id"s, Node{ request_map.at("id"s) });
    return;
}

void RequestHandler::MakeMapDict(json::Dict& r, const json::Node& id) const {
    using namespace std::literals;

    std::ostringstream map;
    RenderMap(map);
    r.emplace("map"s, map.str());
    r.emplace("request_id"s, id);
}

void RequestHandler::RenderMap(std::ostream& out) const {
    
    std::vector<BusPtr> buses;
    std::vector<StopPtr> stops;

    for (const auto& [busname, bus] : *db_.GetBusMap()) {
        buses.emplace_back(bus);
    }

    sort(buses.begin(), buses.end(), [](const BusPtr& lhs, const BusPtr& rhs) {
        return std::lexicographical_compare(lhs->bus_name.begin(), lhs->bus_name.end(), rhs->bus_name.begin(), rhs->bus_name.end());
        });

    for (const auto& [stop_ptr, set_bus] : *db_.GetBusesByStop()) {
        if (!set_bus.empty()) {
            stops.emplace_back(stop_ptr);
        }
    }
    sort(stops.begin(), stops.end(), [](const StopPtr& lhs, const StopPtr& rhs) {
        return std::lexicographical_compare(lhs->stop_name.begin(), lhs->stop_name.end(), rhs->stop_name.begin(), rhs->stop_name.end());
        });

    svg::Document document;
    renderer_.GetMapDocument(document, buses, stops);

    document.Render(out);
}
