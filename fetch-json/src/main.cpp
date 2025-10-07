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

int main(int argc, const char * argv[]) {
    header::map headers = {
        { "content-type", "application/json" },
    };
    
    string      url = "http://localhost:8081/greeting";
    // string      url = "http://localhost:8081/no-reply";

    string      method = "POST";
    auto        body = new object((vector<object*>) {
        new object("firstName", encode("Corey")),
        // new object("lastName", encode("Ferguson"))
        
        new object("fullName", encode("Corey Ferguson"))
    });

    try {
        auto response = request(headers, url, method, stringify(body));
        
        try {
            cout << stringify(response.json()) << endl;
        } catch (json::error& e) {
            cout << response.text() << endl;
        }
    } catch (fetch::error& e) {
        if (e.text().length())
            throw fetch::error(e.status(), e.text(), e.text(), e.headers());
        
        throw e;
    }
}
