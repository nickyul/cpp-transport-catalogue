#include "json_reader.h"

using namespace std::literals;


void JsonReader::MakeCatalogue(catalogue::TransportCatalogue& catalogue) const {
	Array base_requests_arr = document_.GetRoot().AsMap().at("base_requests"s).AsArray();
	ParseStopCoord(base_requests_arr, catalogue);
	ParseStopDistances(base_requests_arr, catalogue);
	ParseBuses(base_requests_arr, catalogue);
}

void JsonReader::ParseStopCoord(const Array& base_requests_arr, catalogue::TransportCatalogue& catalogue) const {
	for (const Node& stop_node : base_requests_arr) {
		Dict stop_map = stop_node.AsMap();
		if (stop_map.at("type"s).AsString() != "Stop"s) {
			continue;
		}
		catalogue.AddStop(stop_map.at("name"s).AsString(), { stop_map.at("latitude"s).AsDouble(), stop_map.at("longitude"s).AsDouble() });
	}
}

void JsonReader::ParseStopDistances(const Array& base_requests_arr, catalogue::TransportCatalogue& catalogue) const {
	for (const Node& stop_node : base_requests_arr) {
		if (stop_node.AsMap().at("type"s).AsString() != "Stop"s) {
			continue;
		}
		StopPtr stop_from = catalogue.GetStop(stop_node.AsMap().at("name"s).AsString());

		for (const auto& dist : stop_node.AsMap().at("road_distances"s).AsMap()) {
			catalogue.AddDistance(stop_from, catalogue.GetStop(dist.first), dist.second.AsInt());
		}
	}
}

void JsonReader::ParseBuses(const Array& base_requests_arr, catalogue::TransportCatalogue& catalogue) const {
	for (const Node& bus_node : base_requests_arr) {
		Dict bus_map = bus_node.AsMap();
		if (bus_map.at("type"s).AsString() != "Bus"s) {
			continue;
		}
		std::vector<const Stop*> stops;

		for (const Node& stop_node : bus_map.at("stops"s).AsArray()) {
			stops.emplace_back(catalogue.GetStop(stop_node.AsString()));
		}
		if (!bus_map.at("is_roundtrip"s).AsBool()) {
			bool first = true;
			for (auto it = bus_map.at("stops"s).AsArray().rbegin(); it != bus_map.at("stops"s).AsArray().rend(); ++it) {
				if (first) {
					first = false;
					continue;
				}
				stops.emplace_back(catalogue.GetStop(it->AsString()));
			}
		}
		catalogue.AddBus(bus_map.at("name"s).AsString(), stops, bus_map.at("is_roundtrip"s).AsBool());
	}
}

svg::Color JsonReader::ParseColor(const Node& color) const {
	using namespace svg;
	if (color.IsString()) {
		return Color(color.AsString());
	}
	else if (color.IsArray()) {
		if (color.AsArray().size() == 3) {
			return Color(Rgb{ static_cast<uint8_t>(color.AsArray()[0].AsInt()),
			static_cast<uint8_t>(color.AsArray()[1].AsInt()),
			static_cast<uint8_t>(color.AsArray()[2].AsInt()) });
		}
		else if (color.AsArray().size() == 4) {
			return Color(Rgba{ static_cast<uint8_t>(color.AsArray()[0].AsInt()),
			static_cast<uint8_t>(color.AsArray()[1].AsInt()),
			static_cast<uint8_t>(color.AsArray()[2].AsInt()),
			color.AsArray()[3].AsDouble() });
		}
	}
	return svg::NoneColor;
}

void JsonReader::MakeBusDict(json::Dict& r, BusStat stat, const json::Dict& request_map) const {
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

void JsonReader::MakeStopDict(json::Dict& r, const catalogue::TransportCatalogue& catalogue, const json::Dict& request_map) const {
	using namespace std::literals;

	if (catalogue.GetStop(request_map.at("name"s).AsString()) == nullptr) {
		r.emplace("request_id"s, Node{ request_map.at("id"s) });
		r.emplace("error_message"s, Node{ "not found"s });
		return;
	}
	if (catalogue.RequestStop(catalogue.GetStop(request_map.at("name"s).AsString())) == nullptr) {
		r.emplace("buses"s, std::vector<Node>{});
		r.emplace("request_id"s, Node{ request_map.at("id"s) });
		return;
	}
	std::vector<std::string> vec;
	for (BusPtr bus : *catalogue.RequestStop(catalogue.GetStop(request_map.at("name"s).AsString()))) {
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

void JsonReader::MakeMapDict(json::Dict& r, const catalogue::TransportCatalogue& catalogue, const MapRenderer& renderer, const json::Node& id) const {
	using namespace std::literals;

	std::ostringstream map_stream;
	svg::Document map_document;
	renderer.GetMapDocument(map_document, catalogue);
	map_document.Render(map_stream);
	r.emplace("map"s, map_stream.str());
	r.emplace("request_id"s, id);
}

RenderSettings JsonReader::ParseSettings() const {
	RenderSettings settings;
	Dict render_settings_map = document_.GetRoot().AsMap().at("render_settings"s).AsMap();

	settings.width_ = render_settings_map.at("width"s).AsDouble();
	settings.height_ = render_settings_map.at("height"s).AsDouble();

	settings.padding_ = render_settings_map.at("padding"s).AsDouble();

	settings.line_width_ = render_settings_map.at("line_width"s).AsDouble();
	settings.stop_radius_ = render_settings_map.at("stop_radius"s).AsDouble();

	settings.bus_label_font_size_ = render_settings_map.at("bus_label_font_size"s).AsDouble();
	settings.bus_label_offset_ = std::make_pair(render_settings_map.at("bus_label_offset"s).AsArray()[0].AsDouble(),
		render_settings_map.at("bus_label_offset"s).AsArray()[1].AsDouble());

	settings.stop_label_font_size_ = render_settings_map.at("stop_label_font_size"s).AsDouble();
	settings.stop_label_offset_ = std::make_pair(render_settings_map.at("stop_label_offset"s).AsArray()[0].AsDouble(),
		render_settings_map.at("stop_label_offset"s).AsArray()[1].AsDouble());

	settings.underlayer_color_ = ParseColor(render_settings_map.at("underlayer_color"s));

	settings.underlayer_width_ = render_settings_map.at("underlayer_width"s).AsDouble();

	for (const auto& color : render_settings_map.at("color_palette"s).AsArray()) {
		settings.color_palette_.emplace_back(ParseColor(color));
	}
	return settings;
}

json::Document JsonReader::GetRequestDocument(const catalogue::TransportCatalogue& catalogue, const MapRenderer& renderer) const {
	using namespace std::literals;

	std::vector<Node> res;
	for (const Node& request_node : document_.GetRoot().AsMap().at("stat_requests"s).AsArray()) {
		Dict request_map = request_node.AsMap();
		if (request_map.at("type"s).AsString() == "Bus"s) {
			BusStat stat = catalogue.RequestBus(request_map.at("name"s).AsString());
			Dict r;
			MakeBusDict(r, stat, request_map);
			res.emplace_back(move(r));
		}
		else if (request_map.at("type"s).AsString() == "Stop"s) {
			Dict r;
			MakeStopDict(r, catalogue, request_map);
			res.emplace_back(move(r));
		}
		else if (request_map.at("type"s).AsString() == "Map"s) {
			Dict r;
			MakeMapDict(r, catalogue, renderer, request_map.at("id"s));
			res.emplace_back(move(r));
		}
	}
	return Document{ Node{res} };

}
