#include "json_reader.h"

using namespace std::literals;

void JsonReader::MakeCatalogue(catalogue::TransportCatalogue& catalogue) const {
	Array base_requests_arr = document_.GetRoot().AsMap().at("base_requests"s).AsArray();
	ParseStopCoord(base_requests_arr, catalogue);
	ParseStopDistances(base_requests_arr, catalogue);
	ParseBuses(base_requests_arr, catalogue);
}

RouteSettings JsonReader::GetRouteSettings() const {
	Dict settings = document_.GetRoot().AsMap().at("routing_settings"s).AsMap();

	RouteSettings result;
	result.bus_velocity = settings.at("bus_velocity"s).AsDouble();
	result.bus_wait_time = settings.at("bus_wait_time"s).AsDouble();
	if (result.bus_velocity < 1 || result.bus_velocity > 1000 || result.bus_wait_time < 1 || result.bus_wait_time > 1000) {
		throw std::invalid_argument("Non correct velocity or bus wait time"s);
	}
	return result;
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

Node JsonReader::MakeBusDict(BusStat stat, const json::Dict& request_map) const {
	using namespace std::literals;
	Builder result;

	if (stat.total_stops == 0) {
		result.StartDict().
			Key("request_id"s).Value(request_map.at("id"s).AsInt()).
			Key("error_message"s).Value("not found"s).
			EndDict();

		return result.Build();
	}

	result.StartDict().
		Key("curvature"s).Value(stat.curvature).
		Key("request_id"s).Value(request_map.at("id"s).AsInt()).
		Key("route_length"s).Value(stat.route_length).
		Key("stop_count"s).Value(static_cast<int>(stat.total_stops)).
		Key("unique_stop_count"s).Value(static_cast<int>(stat.unique_stops)).
		EndDict();

	return result.Build();
}

Node JsonReader::MakeStopDict(const catalogue::TransportCatalogue& catalogue, const json::Dict& request_map) const {
	using namespace std::literals;
	Builder result;

	if (catalogue.GetStop(request_map.at("name"s).AsString()) == nullptr) {
		result.StartDict().
			Key("request_id"s).Value(request_map.at("id"s).AsInt()).
			Key("error_message"s).Value("not found"s).
			EndDict();

		return result.Build();
	}
	if (catalogue.RequestStop(catalogue.GetStop(request_map.at("name"s).AsString())) == nullptr) {
		result.StartDict().
			Key("buses"s).StartArray().EndArray().
			Key("request_id").Value(request_map.at("id"s).AsInt()).
			EndDict();

		return result.Build();
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
	result.StartDict().
		Key("buses"s).Value(bus_names).
		Key("request_id"s).Value(request_map.at("id"s).AsInt()).
		EndDict();

	return result.Build();
}

Node JsonReader::MakeMapDict(const catalogue::TransportCatalogue& catalogue, const MapRenderer& renderer, const json::Node& id) const {
	using namespace std::literals;
	Builder result;

	std::ostringstream map_stream;
	svg::Document map_document;
	renderer.GetMapDocument(map_document, catalogue);
	map_document.Render(map_stream);
	result.StartDict().
		Key("map"s).Value(map_stream.str()).
		Key("request_id"s).Value(id.AsInt()).
		EndDict();
	
	return result.Build();
}

Node JsonReader::MakeRouteDict(const TransportRouter& router, const json::Dict& request_map) const {
	using namespace std::literals;
	Builder result;

	std::optional<std::pair<Router<RouteWeight>::RouteInfo, std::vector<RouteWeight>>> info = router.BuildRoute(request_map.at("from"s).AsString(), request_map.at("to"s).AsString());
	
	if (!info.has_value()) {
		result.StartDict().
			Key("request_id"s).Value(request_map.at("id"s).AsInt()).
			Key("error_message"s).Value("not found"s).
			EndDict();
		return result.Build();
	}

	result.StartDict().
		Key("request_id"s).Value(request_map.at("id"s).AsInt()).
		Key("total_time"s).Value(info.value().first.weight.route_time).
		Key("items"s).StartArray();

	for (const RouteWeight& item_weight : info.value().second) {
		if (item_weight.is_stop) {
			result.StartDict().
				Key("type"s).Value("Wait"s).
				Key("stop_name"s).Value(std::string(item_weight.name)).
				Key("time"s).Value(item_weight.route_time).
				EndDict();
		}
		else {
			result.StartDict().
				Key("type"s).Value("Bus"s).
				Key("bus"s).Value(std::string(item_weight.name)).
				Key("span_count"s).Value(item_weight.span_count).
				Key("time"s).Value(item_weight.route_time).
				EndDict();
		}
	}
	result.EndArray().EndDict();

	return result.Build();
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

json::Document JsonReader::GetRequestDocument(const catalogue::TransportCatalogue& catalogue, const MapRenderer& renderer, const TransportRouter& router) const {
	using namespace std::literals;

	std::vector<Node> res;
	for (const Node& request_node : document_.GetRoot().AsMap().at("stat_requests"s).AsArray()) {
		Dict request_map = request_node.AsMap();
		if (request_map.at("type"s).AsString() == "Bus"s) {
			BusStat stat = catalogue.RequestBus(request_map.at("name"s).AsString());
			res.emplace_back(MakeBusDict(stat, request_map));
		}
		else if (request_map.at("type"s).AsString() == "Stop"s) {
			res.emplace_back(MakeStopDict(catalogue, request_map));
		}
		else if (request_map.at("type"s).AsString() == "Map"s) {
			res.emplace_back(MakeMapDict(catalogue, renderer, request_map.at("id"s)));
		}
		else if (request_map.at("type"s).AsString() == "Route"s) {
			res.emplace_back(MakeRouteDict(router, request_map));
		}
	}
	return Document{ Node{res} };
}
