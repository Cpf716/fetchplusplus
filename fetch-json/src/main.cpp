//
//  main.cpp
//  fetch-json
//
//  Created by Corey Ferguson on 9/2/25.
//

#include "fetch.h"
#include "json.h"

using namespace fetch;
using namespace json;
using namespace std;

enum logging parse_logging(const std::string value) {
    int index = ((map<string, int>) {
        { "none", 1 },
        { "info", 2 },
        { "extended", 3 }
    })[value] - 1;
    
    return index == -1 ? INFO : static_cast<enum logging>(index);
}

int main(int argc, const char* argv[]) {
    auto         logging = argc == 1 ? INFO : parse_logging(argv[1]);
    class logger logger(logging);
    http_client  http(logger);
    
    http.timeout() = 10;
    http.max_redirects() = 10;
    
    header::map headers = {
//         { "content-type", "application/json" },
    };
    
    std::string url = "http://calapi.inadiutorium.cz/api/v0/en/calendars/general-en/today";

//     std::string url = "http://localhost:8080/api/greeting";

    auto body = new object((vector<object*>) {
        new object("firstName", encode("Corey")),
    });

    try {
        http.options(headers, url);
        
        auto response = http.get(headers, url);

//         auto response = http.post(headers, url, stringify(body));
        
        logger.info(stringify(response.json()));
        
        delete body;
    } catch (fetch::error& e) {
        delete body;
        
        throw e;
    }
}
