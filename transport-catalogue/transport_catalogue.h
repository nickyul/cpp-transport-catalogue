#pragma once
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <string>
#include <deque>
#include <stdexcept>
#include <vector>

#include "geo.h"

namespace catalogue {

	struct Stop {
		std::string stop_name;
		double lat;
		double lng;
	};

	struct Bus {
		std::string bus_name;
		std::vector<Stop*> stops;
	};

	using StopMap = std::unordered_map<std::string_view, Stop*>;
	using BusMap = std::unordered_map<std::string_view, Bus*>;

	class TransportCatalogue {
	public:
		void AddStop(const std::string& stop_name, const detail::Coordinates& coordinates);
		void AddBus(const std::string& bus_name, const std::vector<std::string_view> stops);
		Bus RequestBus(std::string_view bus_name) const;
		bool RequestStop(std::set<std::string>& buses, std::string_view stop_name) const;

	private:
		// deque всех остановок
		std::deque<Stop> stops_;
		// мапа [название остановки] = указатель на остановку
		StopMap stopname_to_stop_;
		// deque всех маршрутов
		std::deque<Bus> buses_;
		// мапа [название маршрута] = указатель на маршрут в деке
		BusMap busname_to_bus_;
		std::unordered_map<Stop*, std::set<std::string>> stop_to_buses_on_stop;
	};
}
