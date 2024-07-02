#pragma once
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
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

	json::Document GetRequestDocument(const catalogue::TransportCatalogue& catalogue, const MapRenderer& renderer) const;

private:
	Document document_;

	void ParseStopCoord(const Array& base_requests_arr, catalogue::TransportCatalogue& catalogue) const;

	void ParseStopDistances(const Array& base_requests_arr, catalogue::TransportCatalogue& catalogue) const;

	void ParseBuses(const Array& base_requests_arr, catalogue::TransportCatalogue& catalogue) const;

	svg::Color ParseColor(const Node& color) const;

	void MakeBusDict(json::Dict& r, BusStat stat, const json::Dict& request_map) const;

	void MakeStopDict(json::Dict& r, const catalogue::TransportCatalogue& catalogue, const json::Dict& request_map) const;

	void MakeMapDict(json::Dict& r, const catalogue::TransportCatalogue& catalogue, const MapRenderer& renderer, const json::Node& id) const;
};
