#include "stat_reader.h"
#include <iomanip>

using namespace std::string_literals;
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

void output::ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
    std::ostream& output) {
    auto colon_space = request.find(' ');
    if (colon_space == request.npos) {
		output << "Can't read the request! \n"s;
        return;
    }
	
	std::string_view command = request.substr(0, colon_space);
	std::string_view request_id = request.substr(colon_space + 1);

	if (command == "Stop"s) {
		std::set<std::string> buses_on_stop;
		if (transport_catalogue.RequestStop(buses_on_stop, request_id)) {
			output::StopStatPrint(buses_on_stop, request_id, output);
		}
		else {
			output << "Stop "s << request_id << ": not found \n"s;
		}
	}

	else if (command == "Bus"s) {
		Bus bus = transport_catalogue.RequestBus(request_id);
		if (bus.stops.empty()) {
			output << "Bus "s << request_id << ": not found \n"s;
			return;
		}
		output::BusStatPrint(bus, output);
	}

	else {
		output << "Can't read the command! \n"s;
		return;
	}

}


void output::StopStatPrint(const std::set<std::string>& buses_on_stop, std::string_view request_id, std::ostream& output) {
	if (buses_on_stop.empty()) {
		output << "Stop "s << request_id << ": no buses \n"s;
		return;
	}
	output << "Stop "s << request_id << ": buses ";
	for (std::string_view bus : buses_on_stop) {
		output << bus << " "s;
	}
	output << "\n";
}

void output::BusStatPrint(const Bus& bus, std::ostream& output) {
	double distance = 0;
	std::unordered_set<std::string_view, unique_names_hasher> unique_stops_names;

	// Compute distance
	detail::Coordinates previous_coord{ bus.stops[0]->lat, bus.stops[0]->lng };
	bool first_iter = true;
	for (const Stop* stop : bus.stops) {
		if (first_iter) {
			first_iter = false;
			unique_stops_names.insert(stop->stop_name);
			continue;
		}
		distance += detail::ComputeDistance(previous_coord, detail::Coordinates{ stop->lat, stop->lng });
		previous_coord = { stop->lat, stop->lng };
		unique_stops_names.insert(stop->stop_name);
	}
	output << "Bus "s << bus.bus_name << ": "s << bus.stops.size() << " stops on route, "s << unique_stops_names.size() << " unique stops, "s << std::setprecision(6) << distance << " route length \n";
}
