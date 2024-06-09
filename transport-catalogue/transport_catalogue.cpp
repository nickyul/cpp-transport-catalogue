#include "transport_catalogue.h"

using namespace catalogue;

struct unique_names_hasher {
	size_t operator()(const std::string_view& stop_name) const {
		double hash = 0;
		for (int i = 0; i < stop_name.size(); ++i) {
			hash += stop_name[i] * std::pow(37, i);
		}
		return static_cast<size_t>(hash);
	}
};

void TransportCatalogue::AddStop(const std::string& stop_name, const detail::Coordinates& coordinates) {
	Stop stop{ stop_name, coordinates.lat, coordinates.lng };
	stops_.push_back(std::move(stop));
	stopname_to_stop_[stops_.back().stop_name] = &stops_.back();
}

void TransportCatalogue::AddDistance(const Stop* stop_from, const Stop* stop_to, int distance) {
	pair_stop_to_distance_[{stop_from, stop_to}] = distance;
}


void TransportCatalogue::AddBus(const std::string& bus_name, const std::vector<const Stop*> stops) {
	const Bus bus{ bus_name, std::move(stops) };
	buses_.push_back(std::move(bus));
	busname_to_bus_[buses_.back().bus_name] = &buses_.back();

	for (const Stop* stop : buses_.back().stops) {
		bus_by_stop_[stop].insert(&buses_.back());
	}
}

BusStat TransportCatalogue::RequestBus(std::string_view bus_name) const {
	// if bus exist
	if (busname_to_bus_.count(bus_name)) {
		const Bus* bus = busname_to_bus_.at(bus_name);
		BusStat stats;
		std::unordered_set<std::string_view, unique_names_hasher> unique_stops_names;

		double geo_distance = 0;
		// Compute distance
		detail::Coordinates previous_coord{ bus->stops[0]->coordinates };
		const Stop* previous_stop = bus->stops[0];
		bool first_iter = true;
		for (const Stop* stop : bus->stops) {
			if (first_iter) {
				first_iter = false;
				unique_stops_names.insert(stop->stop_name);
				continue;
			}
			geo_distance += detail::ComputeDistance(previous_coord, stop->coordinates);
			stats.route_length += GetDistance(previous_stop, stop);
			previous_stop = stop;
			previous_coord = stop->coordinates;
			unique_stops_names.insert(stop->stop_name);
		}
		stats.unique_stops = unique_stops_names.size();
		stats.total_stops = bus->stops.size();
		stats.curvature = stats.route_length / geo_distance;
		return stats;
	}
	return {};
}

std::unordered_set<const Bus*> TransportCatalogue::RequestStop(const Stop* stop) const {
	if (bus_by_stop_.count(stop)) {
		return bus_by_stop_.at(stop);
	}
	return {};
}

const Stop* TransportCatalogue::GetStop(std::string_view stop_name) const {
	if (stopname_to_stop_.count(stop_name)) {
		return stopname_to_stop_.at(stop_name);
	}
	return nullptr;
}

const Bus* TransportCatalogue::GetBus(std::string_view bus_name) const {
	if (busname_to_bus_.count(bus_name)) {
		return busname_to_bus_.at(bus_name);
	}
	return nullptr;
}

int TransportCatalogue::GetDistance(const Stop* stop_from, const Stop* stop_to) const {
	if (pair_stop_to_distance_.count({stop_from, stop_to})) {
		return pair_stop_to_distance_.at({ stop_from, stop_to });
	}
	if (pair_stop_to_distance_.count({ stop_to, stop_from })) {
		return pair_stop_to_distance_.at({ stop_to, stop_from });
	}

	return 0;
}
