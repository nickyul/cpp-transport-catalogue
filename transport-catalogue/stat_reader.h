#pragma once
#include <cmath>
#include <iosfwd>

#include "transport_catalogue.h"

namespace catalogue {
    namespace output {

        void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output);

        void StopStatPrint(const std::set<std::string>& buses_on_stop, std::string_view request_id, std::ostream& output);

        void BusStatPrint(const Bus& bus, std::ostream& output);
    }
}
