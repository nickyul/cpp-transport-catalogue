#include <iostream>
#include <string>

#include "request_handler.h"
#include "map_renderer.h"
#include "json_reader.h"

using namespace std;
using namespace catalogue;
using namespace json;

int main() {
    TransportCatalogue catalogue;
    JsonReader json_reader(cin);
    json_reader.MakeCatalogue(catalogue);
    RenderSettings settings = json_reader.ParseSettings();

    RequestHandler request_handler(catalogue, json_reader, settings);
    
    Document doc_out = request_handler.GetRequestDocument();

    Print(doc_out, cout);

    return 0;
}
