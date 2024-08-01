#pragma once

#include "geo.h"

#include <string>
#include <vector>
#include <unordered_map>

struct Stop {
	std::string stop_name;
	catalogue::detail::Coordinates coordinates;
};

struct Bus {
	std::string bus_name;
	std::vector<const Stop*> stops;
	bool is_roundtrip;
};

using StopMap = std::unordered_map<std::string_view, const Stop*>;
using BusMap = std::unordered_map<std::string_view, const Bus*>;
using StopPtr = const Stop*;
using BusPtr = const Bus*;

struct BusStat {
	size_t total_stops = 0;
	size_t unique_stops = 0;
	double route_length = 0.;
	double curvature = 0;
};

struct RouteSettings {
	double bus_wait_time = 0.;
	double bus_velocity = 0.;
};

struct StopId {
	size_t input_id = 0;
	size_t output_id = 0;
};

using StopMap = std::unordered_map<std::string_view, const Stop*>;
using BusMap = std::unordered_map<std::string_view, const Bus*>;
using StopPtr = const Stop*;
using BusPtr = const Bus*;
