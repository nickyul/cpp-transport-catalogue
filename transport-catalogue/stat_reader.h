#pragma once
#include <cmath>
#include <iosfwd>

#include "transport_catalogue.h"

namespace catalogue {
    namespace output {

        void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output);
    }
}
