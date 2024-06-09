#pragma once
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <string>
#include <deque>
#include <vector>

#include "geo.h"


namespace catalogue {

	struct Stop {
		std::string stop_name;
		detail::Coordinates coordinates;
	};

	struct Bus {
		std::string bus_name;
		std::vector<const Stop*> stops;
	};

	struct BusStat {
		size_t total_stops = 0;
		size_t unique_stops = 0;
		double route_length = 0.;
		double curvature = 0;
	};

	struct pair_stops_hasher {
		size_t operator()(const std::pair<const Stop*, const Stop*>& pair) const {
			std::hash<const void*> hasher;
			return hasher(pair.first) + hasher(pair.second)*37;
		}
	};

	using StopMap = std::unordered_map<std::string_view, const Stop*>;
	using BusMap = std::unordered_map<std::string_view, const Bus*>;

	class TransportCatalogue {
	public:
		void AddStop(const std::string& stop_name, const detail::Coordinates& coordinates);
		void AddDistance(const Stop* stop_from, const Stop* stop_to, int distance);
		void AddBus(const std::string& bus_name, const std::vector<const Stop*> stops);
		BusStat RequestBus(std::string_view bus_name) const;
		// возвращайте константный указатель на объект по имени остановки
		std::unordered_set<const Bus*> RequestStop(const Stop* stop) const;
		const Stop* GetStop(std::string_view stop_name) const;
		const Bus* GetBus(std::string_view bus_name) const;
		int GetDistance(const Stop* stop_from, const Stop* stop_to) const;

	private:
		// deque всех остановок
		std::deque<Stop> stops_;
		// мапа [название остановки] = указатель на остановку
		StopMap stopname_to_stop_;
		// deque всех маршрутов
		std::deque<Bus> buses_;
		// мапа [название маршрута] = указатель на маршрут в деке
		BusMap busname_to_bus_;

		std::unordered_map<const Stop*, std::unordered_set<const Bus*>> bus_by_stop_;

		std::unordered_map<std::pair<const Stop*, const Stop*>, int, pair_stops_hasher> pair_stop_to_distance_;
	};
}
