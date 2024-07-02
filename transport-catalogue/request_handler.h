#pragma once
#include "transport_catalogue.h"
#include "svg.h"
#include "map_renderer.h"
#include "json_reader.h"

#include <algorithm>
#include <optional>
#include <unordered_set>
#include <sstream>

using namespace catalogue;

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const TransportCatalogue& db, const JsonReader& json_reader, const MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::unordered_set<const Bus*>* GetBusesByStop(const std::string_view& stop_name) const;

    void RenderMap(std::ostream& out) const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& db_;
    const JsonReader& json_reader_;
    const MapRenderer& renderer_;
};
