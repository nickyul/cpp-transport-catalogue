#pragma once
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"

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

	void MakeCatalogue(catalogue::TransportCatalogue& cat) const;

	Array GetRequestArray() const;

	RenderSettings ParseSettings() const;


private:
	Document document_;

	void ParseStopCoord(const Array& base_requests_arr, catalogue::TransportCatalogue& cat) const;

	void ParseStopDistances(const Array& base_requests_arr, catalogue::TransportCatalogue& cat) const;

	void ParseBuses(const Array& base_requests_arr, catalogue::TransportCatalogue& cat) const;

	svg::Color ParseColor(const Node& color) const;
};