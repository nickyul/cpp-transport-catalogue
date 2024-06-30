#pragma once
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <string>
#include <string_view>
#include <deque>
#include <vector>

#include "geo.h"
#include "domain.h"


namespace catalogue {

	struct pair_stops_hasher {
		size_t operator()(const std::pair<const Stop*, const Stop*>& pair) const {
			std::hash<const void*> hasher;
			return hasher(pair.first) + hasher(pair.second) * 37;
		}
	};

	struct unique_names_hasher {
		size_t operator()(const std::string_view& stop_name) const {
			double hash = 0;
			for (size_t i = 0; i < stop_name.size(); ++i) {
				hash += stop_name[i] * std::pow(37, i);
			}
			return static_cast<size_t>(hash);
		}
	};

	class TransportCatalogue {
	public:
		void AddStop(const std::string& stop_name, const detail::Coordinates& coordinates);
		void AddDistance(StopPtr stop_from, StopPtr stop_to, int distance);
		void AddBus(const std::string& bus_name, const std::vector<StopPtr> stops, bool is_roundtrip);
		BusStat RequestBus(std::string_view bus_name) const;
		const std::unordered_set<BusPtr>* RequestStop(StopPtr stop) const;
		StopPtr GetStop(std::string_view stop_name) const;
		BusPtr GetBus(std::string_view bus_name) const;
		const std::unordered_map<StopPtr, std::unordered_set<BusPtr>>* GetBusesByStop() const;
		const BusMap* GetBusMap() const;
		int GetDistance(StopPtr stop_from, StopPtr stop_to) const;

	private:
		// deque всех остановок
		std::deque<Stop> stops_;
		// мапа [название остановки] = указатель на остановку
		StopMap stopname_to_stop_;
		// deque всех маршрутов
		std::deque<Bus> buses_;
		// мапа [название маршрута] = указатель на маршрут в деке
		BusMap busname_to_bus_;

		std::unordered_map<StopPtr, std::unordered_set<BusPtr>> bus_by_stop_;

		std::unordered_map<std::pair<StopPtr, StopPtr>, int, pair_stops_hasher> pair_stop_to_distance_;
	};
}
