#include "stat_reader.h"
#include <iomanip>

using namespace std::string_literals;
using namespace catalogue;

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
		
		const Stop* stop = transport_catalogue.GetStop(request_id);
		if (stop == nullptr) {
			output << "Stop "s << request_id << ": not found\n"s;
			return;
		}
		std::unordered_set<const Bus*> buses_on_stop = transport_catalogue.RequestStop(stop);
		if (buses_on_stop.empty()) {
			output << "Stop "s << request_id << ": no buses\n"s;
			return;
		}
		else {
			std::vector<std::string> sort_bus;
			for (const Bus* bus : buses_on_stop) {
				sort_bus.push_back(bus->bus_name);
			}
			std::sort(sort_bus.begin(), sort_bus.end());
			output << "Stop "s << request_id << ": buses ";
			for (const std::string_view s_bus : sort_bus) {
				output << s_bus << " "s;
			}
			output << "\n";
		}
	}

	else if (command == "Bus"s) {
		BusStat stats = transport_catalogue.RequestBus(request_id);
		if (!stats.route_length) {
			output << "Bus "s << request_id << ": not found \n"s;
			return;
		}
		output << "Bus "s << request_id << ": "s << stats.total_stops << " stops on route, "s << 
			stats.unique_stops << " unique stops, "s << std::setprecision(6) << stats.route_length << " route length\n";
	}

	else {
		output << "Can't read the command!\n"s;
		return;
	}
}
