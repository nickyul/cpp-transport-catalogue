#include "transport_router.h"

TransportRouter::TransportRouter(const catalogue::TransportCatalogue& catalogue, const RouteSettings& settings)
	: settings_(settings), graph_(2 * catalogue.GetStops()->size()) {
	
	SetStopsGraph(catalogue);
	SetBusesGraph(catalogue);

	router_ptr_ = std::make_unique<Router<RouteWeight>>(Router(graph_));
}

std::optional<std::pair<Router<RouteWeight>::RouteInfo, std::vector<RouteWeight>>> TransportRouter::BuildRoute(std::string_view from, std::string_view to) const {
	std::vector<RouteWeight> route_items;
	std::optional<Router<RouteWeight>::RouteInfo> route_info = router_ptr_.get()->BuildRoute(stop_name_to_id_.at(from).input_id, stop_name_to_id_.at(to).input_id);

	if (route_info.has_value()) {
		for (size_t egde_id : route_info.value().edges) {
			route_items.emplace_back(graph_.GetEdge(egde_id).weight);
		}

		return std::make_pair(route_info.value(), route_items);
	}

	return std::nullopt;
}

void TransportRouter::SetStopsGraph(const catalogue::TransportCatalogue& catalogue) {
	StopId stop_id{ 0 , 1 };

	for (const Stop& stop : *catalogue.GetStops()) {
		stop_name_to_id_.insert({ stop.stop_name, stop_id });
		graph_.AddEdge({ stop_id.input_id, stop_id.output_id, { true, stop.stop_name , settings_.bus_wait_time, 0} });

		stop_id.input_id += 2;
		stop_id.output_id += 2;
	}
}

void TransportRouter::SetBusesGraph(const catalogue::TransportCatalogue& catalogue) {

	for (const Bus& bus : *catalogue.GetBuses()) {
		if (bus.is_roundtrip) {
			for (size_t i = 0; i < bus.stops.size() - 1; ++i) {

				StopPtr curr_stop = bus.stops[i];
				double distance = 0;
				int span_count = 0;
				
				for (size_t j = i + 1; j < bus.stops.size(); ++j) {
					StopPtr iter_stop = bus.stops[j];
					distance += static_cast<double>(catalogue.GetDistance(bus.stops[j - 1], iter_stop));
					double route_time = distance / (settings_.bus_velocity * TRANSLATE_TO_M_MIN);
					graph_.AddEdge({ stop_name_to_id_.at(curr_stop->stop_name).output_id, stop_name_to_id_.at(iter_stop->stop_name).input_id, { false, bus.bus_name, route_time, ++span_count} });
				}
			}
		}
		else {
			for (size_t i = 0; i < (bus.stops.size() - 1) / 2; ++i) {
				StopPtr curr_stop = bus.stops[i];

				double distance_forward = 0;
				double distance_backward = 0;
				int span_count_forward = 0;
				int span_count_backward = 0;

				for (size_t j = i + 1; j < (bus.stops.size() + 1) / 2; ++j) {
					StopPtr iter_stop = bus.stops[j];

					distance_forward += static_cast<double>(catalogue.GetDistance(bus.stops[j - 1], iter_stop));
					distance_backward += static_cast<double>(catalogue.GetDistance(iter_stop, bus.stops[j - 1]));
					
					double route_time_forward = distance_forward / (settings_.bus_velocity * TRANSLATE_TO_M_MIN);
					double route_time_backward = distance_backward / (settings_.bus_velocity * TRANSLATE_TO_M_MIN);

					graph_.AddEdge({ stop_name_to_id_.at(curr_stop->stop_name).output_id, stop_name_to_id_.at(iter_stop->stop_name).input_id, { false, bus.bus_name, route_time_forward, ++span_count_forward} });
					graph_.AddEdge({ stop_name_to_id_.at(iter_stop->stop_name).output_id, stop_name_to_id_.at(curr_stop->stop_name).input_id, { false, bus.bus_name, route_time_backward, ++span_count_backward} });
				}
			}
		}
	}
}
