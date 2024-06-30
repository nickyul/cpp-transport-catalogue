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

    Array stat_requests_arr = json_reader_.GetRequestArray();
    std::vector<Node> res;
    for (const Node& request_node : stat_requests_arr) {
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

void RequestHandler::RenderPolyline(svg::Document& doc, const std::vector<std::pair<BusPtr, int>>& buses_palette, const SphereProjector& proj) const {

    for (const auto& [bus, palette] : buses_palette) {
        std::vector<detail::Coordinates> bus_stops;
        for (StopPtr stop : bus->stops) {
            bus_stops.emplace_back(stop->coordinates);
        }

        svg::Polyline line;
        bool bus_empty = true;

        for (const detail::Coordinates& point : bus_stops) {
            bus_empty = false;
            line.AddPoint(proj(point));
        }

        if (!bus_empty) {
            renderer_.SetLine(line, palette);
            doc.Add(line);
        }
        bus_stops.clear();
    }
}

void RequestHandler::RenderBusName(svg::Document& doc, const std::vector<std::pair<BusPtr, int>>& buses_palette, const SphereProjector& proj) const {
    using namespace std::literals;

    for (const auto& [bus, palette] : buses_palette) {

        if (bus->stops.size() > 0) {            
            svg::Text bus_name_first;
            svg::Text bus_help_first;

            bus_name_first.SetData(bus->bus_name)
                .SetPosition(proj(bus->stops[0]->coordinates))
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold"s);
            bus_help_first.SetData(bus->bus_name)
                .SetPosition(proj(bus->stops[0]->coordinates))
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold"s);
            renderer_.SetBusText(bus_name_first, bus_help_first, palette);
            doc.Add(bus_help_first);
            doc.Add(bus_name_first);

            if (!(bus->is_roundtrip) && bus->stops[(bus->stops.size()/2)]->stop_name != bus->stops[0]->stop_name) {
                svg::Text bus_name_second;
                svg::Text bus_help_second;
                bus_name_second.SetData(bus->bus_name)
                    .SetPosition(proj(bus->stops[(bus->stops.size() / 2)]->coordinates))
                    .SetFontFamily("Verdana"s)
                    .SetFontWeight("bold"s);
                bus_help_second.SetData(bus->bus_name)
                    .SetPosition(proj(bus->stops[(bus->stops.size() / 2)]->coordinates))
                    .SetFontFamily("Verdana"s)
                    .SetFontWeight("bold"s);
                renderer_.SetBusText(bus_name_second, bus_help_second, palette);
                doc.Add(bus_help_second);
                doc.Add(bus_name_second);
            }
        }
    }
}

void RequestHandler::RenderStopCircle(svg::Document& doc, const std::vector<StopPtr>& stops, const SphereProjector& proj) const {
    using namespace std::literals;

    for (StopPtr stop : stops) {
        svg::Circle stop_circle;
        stop_circle.SetCenter(proj(stop->coordinates))
            .SetFillColor("white"s);
        renderer_.SetStopCircle(stop_circle);
        doc.Add(stop_circle);
    }
}

void RequestHandler::RenderStopName(svg::Document& doc, const std::vector<StopPtr>& stops, const SphereProjector& proj) const {
    using namespace std::literals;

    for (StopPtr stop : stops) {
        svg::Text stop_name;
        svg::Text stop_help;
        stop_name.SetPosition(proj(stop->coordinates))
            .SetFontFamily("Verdana"s)
            .SetData(stop->stop_name)
            .SetFillColor("black"s);
        stop_help.SetPosition(proj(stop->coordinates))
            .SetFontFamily("Verdana"s)
            .SetData(stop->stop_name);
        renderer_.SetStopText(stop_name, stop_help);
        doc.Add(stop_help);
        doc.Add(stop_name);
    }
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
    svg::Document doc;

    std::vector<detail::Coordinates> geo_coords;
    std::vector<BusPtr> buses;
    std::vector<std::pair<BusPtr, int>> buses_palette;
    std::vector<StopPtr> stops;

    int palette_size = renderer_.GetPaletteSize();
    int palette_index = 0;

    for (const auto& [busname, bus] : *db_.GetBusMap()) {
        buses.emplace_back(bus);
    }

    sort(buses.begin(), buses.end(), [](const BusPtr& lhs, const BusPtr& rhs) {
        return std::lexicographical_compare(lhs->bus_name.begin(), lhs->bus_name.end(), rhs->bus_name.begin(), rhs->bus_name.end());
        });

    const std::unordered_map<StopPtr, std::unordered_set<BusPtr>>* stops_map = db_.GetBusesByStop();
    for (const auto& [stop_ptr, set_bus] : *stops_map) {
        if (!set_bus.empty()) {
            stops.emplace_back(stop_ptr);
        }
    }
    sort(stops.begin(), stops.end(), [](const StopPtr& lhs, const StopPtr& rhs) {
        return std::lexicographical_compare(lhs->stop_name.begin(), lhs->stop_name.end(), rhs->stop_name.begin(), rhs->stop_name.end());
        });

    for (StopPtr stop : stops) {
        geo_coords.emplace_back(stop->coordinates);
    }

    const SphereProjector proj = renderer_.GetSphereProjector(geo_coords);

    for (BusPtr bus : buses) {

        if (bus->stops.size() > 0) {

            buses_palette.emplace_back(std::make_pair(bus, palette_index));
            palette_index++;

            if (palette_index == palette_size) {
                palette_index = 0;
            }
        }
    }


    RenderPolyline(doc, buses_palette, proj);

    RenderBusName(doc, buses_palette, proj);

    RenderStopCircle(doc, stops, proj);

    RenderStopName(doc, stops, proj);

    doc.Render(out);
}