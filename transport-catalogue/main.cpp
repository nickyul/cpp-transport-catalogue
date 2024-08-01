#include <iostream>
#include <string>

#include "map_renderer.h"
#include "json_reader.h"
#include "request_handler.h"
#include "transport_router.h"

using namespace std;
using namespace catalogue;
using namespace json;

int main() {
    TransportCatalogue catalogue;
    JsonReader json_reader(cin);
    json_reader.MakeCatalogue(catalogue);

    RenderSettings settings = json_reader.ParseSettings();
    MapRenderer renderer(settings);

    TransportRouter transport_router(catalogue, json_reader.GetRouteSettings());

    
    RequestHandler request_handler(catalogue, json_reader, renderer);
    
    Document doc_out = json_reader.GetRequestDocument(catalogue, renderer, transport_router);
    Print(doc_out, cout);
    
    return 0;
}
