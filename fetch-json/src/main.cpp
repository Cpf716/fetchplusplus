//
//  main.cpp
//  fetch-json
//
//  Created by Corey Ferguson on 9/2/25.
//

#include "fetch.h"

using namespace fetch;
using namespace std;

// Non-Member Functions

logging parse_logging(map<string, string> options)
{
    auto it = options.find("-l");

    if (it == options.end())
        it = options.find("--log");

    if (it == options.end())
        return LOG_SOME;

    int index = ((map<string, int>){
                    {"none", 1},
                    {"some", 2},
                    {"more", 3},
                    {"most", 4}})[tolowerstr((*it).second)] -
                1;

    if (index == -1)
    {
        cout << "Option not found: " << (*it).second << endl;

        return LOG_SOME;
    }

    return static_cast<enum logging>(index);
}

int main(int argc, const char *argv[])
{
    logging level = parse_logging(
        options(argc, argv));

    class logger logger(level);

    http_client http(&logger);

    header::map headers;

    cout << "Fetching all vehicle makes from the NHTSA...\n";

    class url url("https://vpic.nhtsa.dot.gov/api/vehicles/getallmakes");

    url.params()["format"] = string("xml");

    try
    {
        http.options(headers, url.str());
    }
    catch (fetch::error &e)
    {
        cerr << e.what() << endl;
    }

    try
    {
        auto response = http.get(headers, url.str());

        auto start = std::chrono::steady_clock::now();

        unique_ptr<xml::element> xml_ptr(response.xml());

        auto results = xml_ptr.get()->find("Results");

        for (auto make : results->children())
        {
            string make_name = make->find("Make_Name")->strin();

            cout << xml::unescape(make_name);
        }

        cout << endl;
        cout << (std::chrono::duration<double>(std::chrono::steady_clock::now() - start)
                     .count() *
                 1000)
             << " ms\n";
    }
    catch (fetch::error &e)
    {
        throw e;
    }
}
