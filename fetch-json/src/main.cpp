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

    header::map h1, h2, h3;

    // Initialize null exception_ptr
    exception_ptr e;

    // Initialize latch with the number of requests
    cpp17_latch latch(2);

    // cpp17_latch latch(3);

    auto cb = [&e, &latch](class response response, fetch::error error) {
        try {
            // Throw error so it can be captured
            if (!response.ok())
                throw error;
        } catch (...) {
            e = current_exception();
        }

        latch.count_down();
    };

    try {
        http.get(h1, "http://localhost:8080/api/ping", cb);

        unique_ptr<object> body = make_unique<object>((vector<object*>) {
            new object("firstName", encode("$hy"))
        });

        http.post(h2, "http://localhost:8080/api/greeting", body.get(), cb);

        // Fetch undefined path
        // http.get(h3, "http://localhost:8080/api/no-path", cb);
        
        // Block the main thread until all asynchronous requests have returned
        latch.wait();

        // Throw error on main thread if one or more requests returned an error
        if (e) {
            rethrow_exception(e);
        }
    } catch (fetch::error& e) {
        throw e;
    }
}
