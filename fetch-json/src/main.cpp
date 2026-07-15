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

http_client http;

// Non-Member Functions

logging parse_logging(const std::string value) {
    int index = ((map<string, int>) {
        { "none", 1 },
        { "some", 2 },
        { "more", 3 },
    })[tolowerstr(value)] - 1;

    if (index == -1) {
        cout << "Option '" + value + "' not recognized\n";

        return LOG_SOME;
    }

    return static_cast<enum logging>(index);
}

void set_logging(map<string, string> options) {
    auto it = options.find("-l");

    if (it == options.end())
        it = options.find("-log");

    http.set_logging(
        it == options.end() ?
            LOG_SOME :
            parse_logging((* it).second)
    );

    it = options.find("--tls");

    if (it != options.end())
        http.set_logging(LOG_MORE_TLS);
}

int main(int argc, const char* argv[]) {
    map<string, string> opts = options(argc, argv);

    set_logging(opts);

    http.timeout() = 60;

    header::map headers;

    try {
        cout << "Fetching all vehicle makes...\n";

        auto response = http.get(headers, "https://vpic.nhtsa.dot.gov/api/vehicles/getallmakes?format=json");
        auto json = response.json();
        
        cout << "Results: " << stringify(json->get("Results"), true) << endl;
    } catch (fetch::error& e) {
        throw e;
    }
}
