#include "transport_catalogue.h"

using namespace catalogue;

void TransportCatalogue::AddStop(const std::string& stop_name, const detail::Coordinates& coordinates){
	Stop stop{ stop_name, coordinates.lat, coordinates.lng };
	stops_.push_back(std::move(stop));
	stopname_to_stop_[stops_.back().stop_name] = &stops_.back();;
}

void TransportCatalogue::AddBus(const std::string& bus_name, const std::vector<std::string_view> stops){
	std::vector<Stop*> bus_stops;
	for (std::string_view stop : stops) {
		if (stopname_to_stop_.count(stop)) {
			bus_stops.push_back(stopname_to_stop_.at(stop));

			stop_to_buses_on_stop[stopname_to_stop_.at(stop)].insert(bus_name);
		}
		else {
			throw std::invalid_argument("Invalid stop on bus " + bus_name);
		}
	}
	Bus bus{ std::move(bus_name), bus_stops };
	buses_.push_back(std::move(bus));
	busname_to_bus_[buses_.back().bus_name] = &buses_.back();
}


Bus TransportCatalogue::RequestBus(std::string_view bus_name) const {
	// if bus exist
	if (busname_to_bus_.count(bus_name)) {
		return *busname_to_bus_.at(bus_name);
	}
	return {};
}

bool TransportCatalogue::RequestStop(std::set<std::string>& buses, std::string_view stop_name) const {
	if (stopname_to_stop_.count(stop_name)) {
		Stop* stop = stopname_to_stop_.at(stop_name);
		if (stop_to_buses_on_stop.count(stop)) {
			buses = stop_to_buses_on_stop.at(stop);
			return true;
		}
	}
	else {
		return false;
	}
	return true;
}
