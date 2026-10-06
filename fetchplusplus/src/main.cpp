//
//  main.cpp
//  fetchplusplus
//
//  Created by Corey Ferguson on 9/2/25.
//

#include "fetch.h"

using namespace fetch;
using namespace std;

// Non-Member Functions

logger_level parse_logging(map<string, string> options) {
    auto it = options.find("-l");

    if (it == options.end())
        it = options.find("--log");

    if (it == options.end())
        return logger_level::some;

    int index = ((map<string, int>) {
        {"none", 1},
        {"some", 2},
        {"more", 3},
        {"most", 4}})
    [tolowerstr((*it).second)] - 1;

    if (index == -1) {
        cout << "Option not found: " << (*it).second << endl;

        return logger_level::some;
    }

    return static_cast<logger_level>(index);
}

int main(int argc, const char *argv[]) {
    logger_level level = parse_logging(
        options(argc, argv)
    );

    class logger logger(level);

    http_client http(&logger);

    header::map headers;

    logger.always("Fetching all vehicle makes from the NHTSA...");

    class url url("https://vpic.nhtsa.dot.gov/api/vehicles/getallmakes");

    url.params()["format"] = string("json");

    try {
        http.options(headers, url.str());
    } catch (fetch::error &e) {
        try {
            unique_ptr<json::object> json_ptr(json::parse(e.what()));

            auto message = json_ptr->get("message");

            logger.error(::unescape(message->str()));
        } catch (json::error& e2) {
            logger.error(e.what());
        }
    }

    auto response = http.get(headers, url.str());
    auto start = std::chrono::steady_clock::now();

    unique_ptr<json::object> json_ptr(response.json());

    auto results = (json::array *)json_ptr->get("Results");

    for (auto make: *results) {
        string make_name = make->get("Make_Name")->value();

        cout << ::unescape(make_name) << endl;
    }

    cout << endl;
    cout << (std::chrono::duration<double>(std::chrono::steady_clock::now() - start)
                .count() * 1000
            )
        << " ms\n";
}
