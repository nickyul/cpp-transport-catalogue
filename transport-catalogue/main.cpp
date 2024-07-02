#include <iostream>
#include <string>

#include "map_renderer.h"
#include "json_reader.h"
#include "request_handler.h"
using namespace std;
using namespace catalogue;
using namespace json;

int main() {
    TransportCatalogue catalogue;
    JsonReader json_reader(cin);
    json_reader.MakeCatalogue(catalogue);
    RenderSettings settings = json_reader.ParseSettings();

    MapRenderer renderer(settings);

    RequestHandler request_handler(catalogue, json_reader, renderer);
    
    Document doc_out = json_reader.GetRequestDocument(catalogue, renderer);

    Print(doc_out, cout);

    return 0;
}
