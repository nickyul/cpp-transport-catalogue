#pragma once

#include <string>
#include <string_view>
#include <memory>
#include <unordered_map>
#include <optional>

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

using namespace graph;

struct RouteWeight {
	bool is_stop = true;
	std::string_view name;
	double route_time = 0.0;
	int span_count = 0;

	bool operator<(const RouteWeight& other) const {
		return route_time < other.route_time;
	}

	bool operator>(const RouteWeight& other) const {
		return route_time > other.route_time;
	}

	RouteWeight operator+(const RouteWeight& other) const {
		return { is_stop, name, route_time + other.route_time, span_count };
	}
};

class TransportRouter {
public:
	TransportRouter(const catalogue::TransportCatalogue& catalogue, const RouteSettings& settings);

	std::optional<std::pair<Router<RouteWeight>::RouteInfo, std::vector<RouteWeight>>> BuildRoute(std::string_view from, std::string_view to) const;

private:
	constexpr static double TRANSLATE_TO_M_MIN = 1000.0 / 60.0;

	void SetStopsGraph(const catalogue::TransportCatalogue& catalogue);
	void SetBusesGraph(const catalogue::TransportCatalogue& catalogue);

	RouteSettings settings_;
	std::unordered_map<std::string_view, StopId> stop_name_to_id_;
	DirectedWeightedGraph<RouteWeight> graph_;
	std::unique_ptr<Router<RouteWeight>> router_ptr_ = nullptr;
};
