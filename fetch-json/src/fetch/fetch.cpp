//
//  fetch.cpp
//  fetch-json
//
//  Created by Corey Ferguson on 9/2/25.
//

#include "fetch.h"

namespace fetch {
    // Non-Member Fields

    const std::string HTTP_VERSION = "HTTP/1.1";

    size_t _max_redirects = 20;
    size_t _timeout = 30;

    // Constructors

    error::error(const size_t status, const std::string status_text, const std::string text, header::map headers, trailer::map trailers) {
        this->_status = status;
        this->_status_text = status_text;
        this->_text = text;
        this->_headers = headers;
        this->_trailers = trailers;
    }

    header::header() {
        this->_set("");
    }

    header::header(const char* value) {
        this->_set(std::string(value));
    }

    header::header(const int value) {
        this->_set(value);
    }

    header::header(const std::string value) {
        this->_set(value);
    }

    header::header(const std::vector<std::string> value) {
        this->_set(value);
    }

    request::request(header::map& headers, const std::string url, const std::string method, const std::string body) {
        // Begin - Map start line
        // Map method
        std::stringstream ss(method + " ");

        // Set cursor to the end
        ss.seekp(0, std::ios::end);

        try {
            // Begin - Map target (path)
            // Begin - Parse host
            this->_url = ::url(url);

            ss << this->url().target() << " ";
            // End - Map target (path)

            // Map protocol
            ss << HTTP_VERSION << "\r\n";
            // End - Map start line

            // Begin - Map request headers
            auto get = [&headers](std::string& key) {
                for (const auto& [_key, value]: headers)
                    if (tolowerstr(_key) == key) {
                        key = _key;

                        return value;
                    }

                return header();
            };

            // Map host
            std::string key = "host",
                        host = get(key);

            // Generate host, if required
            if (host.empty()) {
                host = this->url().host();

                if (this->url().port().typed())
                    host += ":" + std::to_string(this->url().port().value());
            }

            headers.erase(key);
            
            // Map host
            ss << "host: " << host << "\r\n";

            // Generate content-length, if required
            key = "content-length";

            if (get(key).str().empty()) {
                headers.erase(key);

                if (body.length())
                    headers["content-length"] = (int)body.length();
            }       

            // Map request headers
            for (const auto& [key, value]: headers)
                ss << key << ": " << value.str() << "\r\n";

            headers["host"] = host;
            // End - Map request headers

            // Map body
            if (body.length()) {
                ss << "\r\n";
                ss << body << "\r\n";
            }

            ss << "\r\n";

            this->_message = ss.str();

#if LOGGING == LEVEL_DEBUG
            std::cout << this->message() << std::endl;
#endif
        } catch (url::error& e) {
            throw fetch::error(0, "Unknown error");
        }
    }

    response::response() : response(200, "OK") { }

    response::response(const size_t status, const std::string status_text, const std::string text) : response(status, status_text, {}, text) { }

    response::response(const size_t status, const std::string status_text, header::map headers, const std::string text, trailer::map trailers) {
        this->_status = status;
        this->_status_text = status_text;
        this->_headers = headers;
        this->_text = text;
        this->_trailers = trailers;
    }

    response::~response() {
        delete this->_json;
    }

    // Operators

    header::operator int() {
        return this->int_value();
    }

    header::operator std::string() {
        return this->str();
    }

    header::operator std::vector<std::string>() {
        return this->list();
    }

    int header::operator=(const int value) {
        return this->_set(value);
    }

    std::string header::operator=(const std::string value) {
        return this->_set(value);
    }

    std::vector<std::string> header::operator=(const std::vector<std::string> value) {
        return this->_set(value);
    }

    bool header::operator==(const char* value) {
        return this->str() == std::string(value);
    }

    bool header::operator==(const header value) {
        return this->str() == value.str();
    }

    bool header::operator==(const int value) {
        return this->int_value() == value;
    }

    bool header::operator==(const std::string value) {
        return this->str() == value;
    }

    bool header::operator!=(const char* value) {
        return !(*this == value);
    }

    bool header::operator!=(const header value) {
        return !(*this == value);
    }

    bool header::operator!=(const int value) {
        return !(*this == value);
    }

    bool header::operator!=(const std::string value) {
        return !(*this == value);
    }

    // Member Functions

    int header::_set(const int value) {
        this->_int = value;
        this->_str = std::to_string(this->int_value());
        this->_list = { this->str() };

        return this->int_value();
    }

    std::string header::_set(const std::string value) {
        this->_str = value;
        this->_int = parse_int(this->str());
        this->_list = split(this->str(), ", ");

        for (size_t i = 0; i < this->_list.size(); i++)
            this->_list[i] = trim(this->_list[i]);

        return this->str();
    }

    std::vector<std::string> header::_set(const std::vector<std::string> value) {
        this->_list = value;
        this->_str = join(this->list(), ", ");
        this->_int = INT_MIN;

        return this->list();
    }

    header response_t::get(const std::string key) {
        return this->_headers[key];
    }

    header::map response_t::headers() {
        return this->_headers;
    }

    int header::int_value() const {
        return this->_int;
    }

    json::object* response::json() {    
        if (this->_json == NULL) {
            if (!starts_with(this->_headers["content-type"], "application/json"))
                throw json::error("undefined");

            this->_json = json::parse(this->text());
        }

        return this->_json;
    }

    std::vector<std::string> header::list() const {
        return this->_list;
    }

    std::string request::message() const {
        return this->_message;
    }

    size_t response_t::status() const {
        return this->_status;
    }

    std::string response_t::status_text() const {
        return this->_status_text;
    }

    std::string header::str() const {
        return this->_str;
    }

    std::string response_t::text() const {
        return this->_text;
    }

    trailer::map response_t::trailers() {
        return this->_trailers;
    }

    url request::url() {
        return this->_url;
    }

    const char* error::what() const throw() {
        return this->_status_text.c_str();
    }

    // Non-Member Functions

    response _request(header::map& headers, const std::string url, const std::string method, const std::string body, const size_t redirects) {
        class request request(headers, url, method, body);
        class url     url_obj = request.url();

        if (url_obj.protocol() == "https")
            throw fetch::error(0, "Unknown error", "TLS is not supported");

        try {
            std::vector<class host> hosts = dns::lookup(url_obj);

            try {
                tcp_client* client = new tcp_client(hosts[0].ip(), url_obj.port().value());

                try {
                    client->send(request.message());

                    size_t            timeout = fetch::timeout();
                    std::atomic<bool> recved = false;

                    // Begin - Listen for timeout
                    std::thread([timeout, &recved, &client]() {
                        for (size_t i = 0; i < timeout && !recved.load(); i++)
                            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                        if (recved.load())
                            return;

                        recved.store(true);

                        // Sever connection
                        client->close();
                    }).detach();
                    // End - Listen for timeout

                    try {
                        std::string response = client->recv();

#if LOGGING == LEVEL_DEBUG
                        std::cout << response << std::endl;
#endif

                        recved.store(true);
                        client->close();

                        // Server disconnected
                        if (response.empty())
                            throw fetch::error(0, "Unknown error");

                        class response response_obj = parse_response(response);

                        if (response_obj.status() >= 300 && response_obj.status() < 400) {
                            std::string location = response_obj.headers()["location"];

                            if (location.length()) {
                                if (redirects == max_redirects())
                                    throw fetch::error(0, "Unknown error", "Maximum redirects");

// Info
#if LOGGING
                            std::cout << "Redirecting to " << location << std::endl;
#endif

                                return _request(headers, location, method, body, redirects + 1);
                            }
                        }

                        return response_obj;
                    } catch (mysocket::error& e) {
                        if (recved.load())
                            throw fetch::error(0, "Unknown error", "Connection timed out");

                        throw e;
                    }                
                } catch (mysocket::error& e) {
                    client->close();

                    throw e;
                }
            } catch (mysocket::error& e) {
                throw fetch::error(0, "Unknown error", e.what());
            }
        } catch (dns::error& e) {
            throw fetch::error(0, "Unknown error", e.what());
        }
    }

    size_t& max_redirects() {
        return _max_redirects;
    }

    response parse_response(const std::string data) {
        // Begin - Parse response
        std::istringstream iss(data);
        std::string        str;

        getline(iss, str);

        std::vector<std::string> tokens = ::tokens(str);

        // Begin - Parse status and status text
        int status = parse_int(tokens[1]);

        // Merge status text
        for (size_t i = 3; i < tokens.size(); i++)
            tokens[2] += " " + tokens[i];

        std::string status_text = tokens[2];
        // End - Parse status and status text

        // Parse response headers
        header::map headers;

        while (getline(iss, str)) {
            std::vector<std::string> header = split(str, ":");
            
            // Empty line
            if (header.size() == 1)
                break;

            // Case-insensitive
            headers[tolowerstr(header[0])] = trim(str.substr(header[0].length() + 1));
        }

        // Parse response body
        std::ostringstream oss;

        while (getline(iss, str))
            oss << trim_end(str) << "\r\n";

        int          content_length = headers["content-length"];
        std::string  text;
        trailer::map trailers;

        if (content_length == INT_MIN) {
            if (headers["transfer-encoding"] == "chunked") {
                // Parse response body
                iss.str(oss.str());
                iss.clear();

                std::vector<std::string> chunks;

                while (getline(iss, str)) {
                    int size = decimal(split(trim_end(str), ";")[0]);

                    if (size <= 0)
                        break;

                    getline(iss, str);

                    str = trim_end(str);

                    chunks.push_back(str.substr(0, std::min(size, (int)str.length())));
                }

                text = join(chunks, "\r\n");

                // Parse trailers
                while (getline(iss, str)) {
                    std::vector<std::string> trailer = split(str, ":");

                    if (trailer.size() == 1)
                        break;

                    trailers[tolowerstr(trailer[0])] = trim(str.substr(trailer[0].length() + 1));
                }   
            }
        } else
            text = oss.str().substr(0, std::min(content_length, (int)oss.str().length()));

        if (status < 200 || status >= 400)
            throw fetch::error(status, status_text, text, headers, trailers);

        return response(status, status_text, headers, text, trailers);
    }

    response request(header::map& headers, const std::string url, const std::string method, const std::string body) {
        return _request(headers, url, method, body, 0);
    }

    size_t& timeout() {
        return _timeout;
    }
}
