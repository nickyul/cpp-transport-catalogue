#pragma once

#include "transport_catalogue.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "router.h"
#include "transport_router.h"

#include <stdexcept>
#include <optional>
#include <sstream>

using namespace json;

class JsonReader {
public:
	JsonReader() = default;

	JsonReader(Document doc_in)
		: document_(doc_in) {}

	JsonReader(const Node& node)
		: document_(Document{ node }) {}

	JsonReader(std::istream& cin)
		: document_(Load(cin)) {}

	void MakeCatalogue(catalogue::TransportCatalogue& catalogue) const;

	RenderSettings ParseSettings() const;

	RouteSettings GetRouteSettings() const;

	json::Document GetRequestDocument(const catalogue::TransportCatalogue& catalogue, const MapRenderer& renderer, const TransportRouter& route_settings) const;

private:
	Document document_;

	void ParseStopCoord(const Array& base_requests_arr, catalogue::TransportCatalogue& catalogue) const;

	void ParseStopDistances(const Array& base_requests_arr, catalogue::TransportCatalogue& catalogue) const;

	void ParseBuses(const Array& base_requests_arr, catalogue::TransportCatalogue& catalogue) const;

	svg::Color ParseColor(const Node& color) const;

	Node MakeBusDict(BusStat stat, const json::Dict& request_map) const;

	Node MakeStopDict(const catalogue::TransportCatalogue& catalogue, const json::Dict& request_map) const;

	Node MakeMapDict(const catalogue::TransportCatalogue& catalogue, const MapRenderer& renderer, const json::Node& id) const;
	
	Node MakeRouteDict(const TransportRouter& router, const json::Dict& request_map) const;

};
