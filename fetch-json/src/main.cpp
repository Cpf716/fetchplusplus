//
//  main.cpp
//  fetch-json
//
//  Created by Corey Ferguson on 9/2/25.
//

#include "cpp17_latch.h"
#include "fetch.h"
#include "json.h"

using namespace fetch;
using namespace json;
using namespace std;

// Non-Member Fields

logger _logger;

// Non-Member Functions

enum logging parse_logging(const std::string value) {
    int index = ((map<string, int>) {
        { "none", 1 },
        { "info", 2 },
        { "extended", 3 }
    })[value] - 1;
    
    return index == -1 ? INFO : static_cast<enum logging>(index);
}

void set_logging(int argc, const char* argv[]) {
    _logger = logger(argc == 1 ? INFO : parse_logging(argv[1]));
}

int main(int argc, const char* argv[]) {
    set_logging(argc, argv);

    http_client http(_logger);
    header::map headers;

    try {
        cout << "VIN? (optional) ";

        // https://randomvin.com/
        string vin;

        getline(cin, vin);

        if (vin.empty()) {
            auto response = http.get(headers, "https://randomvin.com/getvin.php?type=fake");

            vin = trim_start(response.text());

            headers = { };
        }

        class url url("https://vpic.nhtsa.dot.gov/api/vehicles/decodevin/" + vin);

        url.params()["format"] = string("json");

        cout << "Decoding VIN: " << vin << "...\n";

       // Problem: responses nearing millions of bytes only come through partially
    //    auto response = http.get(headers, "https://vpic.nhtsa.dot.gov/api/vehicles/getallmakes?format=json");

        auto response = http.get(headers, url.str());
        auto json = response.json();
        auto results = (json::array *) json->get("Results");
        
        exception_ptr e;
        
        auto find = [results, &e](string variable) -> string {
            int i = 0;

            while (i < results->size() && decode(results->get(i)->get("Variable")->value()) != variable)
                i++;
            
            try {
                if (i == results->size())
                    throw runtime_error("Decoding failed");
                
                return results->get(i)->get("Value")->value();
            } catch (...) {
                e = current_exception();
            }
            
            return "";
        };

        string year = find("Model Year"),
                make = find("Make"),
                model = find("Model");

        if (e) {
            rethrow_exception(e);
        }
        
        year = decode(year);
        make = decode(make);
        model = decode(model);

        string vehicle = join((vector<string>) { year, make, model }, " ");
        string displacement = find("Displacement (L)"),
                engine = "";

        if (!(e || displacement == null()))
            engine += decode(displacement) + "L";
        
        string cylinders = find("Engine Number of Cylinders");

        if (!(e || cylinders == null())) {
            if (engine.length())
                engine += " ";

            engine += decode(cylinders) + " cyl";
        }

        if (engine.length())
            vehicle += " " + engine;

        cout << "Decoded to a " << vehicle << ".\n";

        // Problem:
        // json::object::_map_keys(...) is not being properly invoked upon parse; therefore _key_map.size() = 0 and I am unable to lookup child objects
    } catch (fetch::error& e) {
        throw e;
    }
}
