//
//  fetch.cpp
//  fetch-json
//
//  Created by Corey Ferguson on 9/2/25.
//

#include "fetch.h"

namespace fetch {
    // Constructors

    error::error(
        const size_t status,
        const std::string status_text,
        const std::string text,
        header::map headers,
        trailer::map trailers
    ) {
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

    response::response(
        const size_t status,
        const std::string status_text,
        header::map headers,
        const std::string text,
        trailer::map trailers
    ) {
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

    bool header::operator==(const int value) {
        return this->int_value() == value;
    }

    bool header::operator==(const std::string value) {
        return this->str() == value;
    }

    bool header::operator==(const header value) {
        return this->str() == value.str();
    }

    bool header::operator!=(const char* value) {
        return !(*this == value);
    }

    bool header::operator!=(const int value) {
        return !(*this == value);
    }

    bool header::operator!=(const std::string value) {
        return !(*this == value);
    }

    bool header::operator!=(const header value) {
        return !(*this == value);
    }

    // Member Functions

    int header::_set(const int value) {
        this->_parsed = true;
        this->_int = value;
        this->_str = std::to_string(this->_int);
        this->_list.push_back(this->str());

        return this->_int;
    }

    std::string header::_set(const std::string value) {
        this->_str = value;
        
        return this->str();
    }

    std::vector<std::string> header::_set(const std::vector<std::string> value) {
        this->_list = value;
        this->_set(join(this->_list, ", "));

        return this->_list;
    }

    header error::get(const std::string key) {
        return this->_headers[key];
    }

    header response::get(const std::string key) {
        return this->_headers[key];
    }

    header::map error::headers() {
        return this->_headers;
    }

    header::map response::headers() {
        return this->_headers;
    }

    int header::int_value() {
        if (!this->_parsed) {
            this->_int = parse_int(this->str());
            this->_parsed = true;
        }

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

    size_t error::status() const {
        return this->_status;
    }

    size_t response::status() const {
        return this->_status;
    }

    std::string error::status_text() const {
        return this->_status_text;
    }

    std::string response::status_text() const {
        return this->_status_text;
    }

    std::string header::str() const {
        return this->_str;
    }

    std::string error::text() const {
        return this->_text;
    }

    std::string response::text() const {
        return this->_text;
    }

    trailer::map fetch::error::trailers() {
        return this->_trailers;
    }

    trailer::map response::trailers() {
        return this->_trailers;
    }

    const char* error::what() const throw() {
        return this->_status_text.c_str();
    }

    // Non-Member Fields

    size_t _timeout = 30;

    // Non-Member Functions

    response request(header::map& headers, const std::string url, const std::string method, const std::string body) {
        // Begin - Map start line
        // Map method
        std::stringstream ss(method + " ");

        // Set cursor to the end
        ss.seekp(0, std::ios::end);

        // Begin - Map target (path)
        // Begin - Parse host
        // Find host
        int start = 0;

        while (start < (int)url.length() - 1 && (url[start] != '/' || url[start + 1] != '/'))
            start++;
        
        start = start == url.length() - 1 ? 0 : start + 2;

        // Find target
        size_t end = start;

        while (end < url.length() && url[end] != '/')
            end++;

        // Parse host
        std::vector<std::string> host = split(url.substr(start, end - start), ":");

        if (host.size() == 1)
            throw fetch::error(0, "port is required");
        
        if (host[0] == "localhost")
            host[0] = "127.0.0.1";
        // End - Parse host

        // Map target
        std::string target = url.substr(end);

        ss << target << " ";
        // End - Map target (path)

        // Map protocol
        ss << "HTTP/1.1\r\n";
        // End - Map start line

        // Begin - Map request headers
        auto get = [&headers](std::string& key) {
            for (const auto& [_key, value]: headers)
                if (tolowers(_key) == key) {
                    key = _key;

                    return value;
                }

            return header();
        };

        // Map host
        std::string key = "host",
                    host_str = get(key);

        // Generate host, if required
        if (host_str.empty())
            host_str = host[0] + ":" + host[1];

        headers.erase(key);

        // Map host
        ss << "host: " << host_str << "\r\n";

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

        headers["host"] = host_str;
        // End - Map request headers

        // Map body
        if (body.length()) {
            ss << "\r\n";
            ss << body << "\r\n";
        }

        ss << "\r\n";

#if LOGGING
       std::cout << ss.str() << std::endl;
#endif

        // Begin - Perform fetch
        std::atomic<bool> recved = false;

        try {
            mysocket::tcp_client* client = new mysocket::tcp_client(host[0], parse_int(host[1]));

            client->send(ss.str());

            size_t timeout = fetch::timeout();

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

            ss.str(client->recv());

            recved.store(true);
            
#if LOGGING
        std::cout << ss.str() << std::endl;
#endif

            client->close();
        } catch (mysocket::error& e) {
            if (recved.load())
                throw fetch::error(0, "Unknown error", "Connection timed out");
            
            throw fetch::error(0, "Unknown error", e.what());
        }

        // Server disconnected
        if (ss.str().empty())
            throw fetch::error(0, "Unknown error");
        // End - Perform fetch

        // Begin - Parse response
        std::string str;

        getline(ss, str);

        std::vector<std::string> tokens = ::tokens(str);

        // Begin - Parse status and status text
        size_t      status = parse_int(tokens[1]);
        std::string status_text = tokens[2];

        // Merge status_text
        for (size_t i = 3; i < tokens.size(); i++)
            status_text += " " + tokens[i];
        // End - Parse status and status text

        // Parse response headers
        header::map response_headers;

        while (getline(ss, str)) {
            std::vector<std::string> header = split(str, ":");
            
            // Empty line
            if (header.size() == 1)
                break;

            // Case-insensitive
            response_headers[tolowers(header[0])] = trim(header[1]);
        }

        // Parse response body
        std::ostringstream oss;

        while (getline(ss, str))
            oss << trim_end(str) << "\r\n";

        int          content_length = response_headers["content-length"];
        std::string  text;
        trailer::map trailers;

        if (content_length == INT_MIN) {
            if (response_headers["transfer-encoding"] == "chunked") {
                // Map response body
                ss.str(oss.str());
                ss.clear();

                std::vector<std::string> chunks;

                while (getline(ss, str)) {
                    int size = decimal(split(trim_end(str), ";")[0]);

                    if (size <= 0)
                        break;

                    getline(ss, str);

                    str = trim_end(str);
                    
                    chunks.push_back(str.substr(0, std::min(size, (int)str.length())));
                }

                text = join(chunks, "\r\n");

                // Map trailers
                while (getline(ss, str)) {
                    std::vector<std::string> trailer = split(str, ":");
                    
                    if (trailer.size() == 1)
                        break;

                    trailers[tolowers(trailer[0])] = trim(trailer[1]);
                }   
            }
        } else
            text = oss.str().substr(0, std::min(content_length, (int)oss.str().length()));

        // Send response
        if (status < 200 || status >= 400)
            throw fetch::error(status, status_text, text, response_headers, trailers);

        return response(status, status_text, response_headers, text, trailers);
    }

    size_t& timeout() {
        return _timeout;
    }
}
