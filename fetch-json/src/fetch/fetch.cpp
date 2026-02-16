//
//  fetch.cpp
//  fetch-json
//
//  Created by Corey Ferguson on 9/2/25.
//

#include "fetch.h"

namespace fetch {
    // Constructors

    error::error(const status_code status, const std::string status_text, const std::string text, header::map headers, trailer::map trailers) {
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
        std::stringstream ss(toupperstr(method) + " ");

        // Set cursor to the end
        ss.seekp(0, std::ios::end);

        try {
            // Begin - Map target (path)
            // Begin - Parse host
            this->_url = ::url(url);

            ss << this->url().target() << " ";
            // End - Map target (path)

            // Map protocol
            ss << http_version() << "\r\n";
            // End - Map start line

            // Begin - Map request headers
            // Map headers to lower
            header::map tmp;
            
            for (const auto& [key, value]: headers)
                tmp[tolowerstr(key)] = value;
            
            headers = tmp;

            // Map host
            std::string host = headers["host"].str();
            
            if (host.empty()) {
                host = this->url().host();

                if (this->url().port().typed())
                    host += ":" + std::to_string((int) this->url().port());
            }

            headers.erase("host");
            
            ss << "host: " << host << "\r\n";

            // Generate content-length, if required
            if (headers["content-length"].str().empty()) {
                if (body.length())
                    headers["content-length"] = (int) body.length();
                else
                    headers.erase("content-length");
            }
            
            headers["user-agent"] = std::string("cpp-fetch/1.0");

            // Map request headers
            for (const auto& [key, value]: headers) {
                std::vector<std::string> tokens = split(key, "-");
                
                for (size_t i = 0; i < tokens.size(); i++)
                    tokens[i] = std::string((char[]) { tokens[i][0], '\0' }) + tokens[i].substr(1);

                for (size_t i = 0; i < tokens.size() - 1; i++)
                    ss << tokens[i] << "-";

                ss << tokens[tokens.size() - 1] << ": " << value.str() << "\r\n";
            }

            headers["Host"] = host;
            // End - Map request headers

            // Map body
            if (body.length()) {
                ss << "\r\n";
                ss << body << "\r\n";
            }

            ss << "\r\n";

            this->_message = ss.str();

            logger::debug(this->message());
        } catch (url::error& e) {
            throw fetch::error(UNKNOWN_ERROR, strstatus(UNKNOWN_ERROR));
        }
    }

    response::response() : response(OK, strstatus(OK)) { }

    response::response(const status_code status, const std::string status_text, const std::string text) : response(status, status_text, {}, text) { }

    response::response(const status_code status, const std::string status_text, header::map headers, const std::string text, trailer::map trailers) {
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
        this->_list = split(this->str(), ",");

        for (size_t i = 0; i < this->_list.size(); i++)
            this->_list[i] = trim(this->_list[i]);

        return this->str();
    }

    std::vector<std::string> header::_set(const std::vector<std::string> value) {
        this->_list = value;
        this->_str = join(this->list(), ",");
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
        if (this->_json == NULL)
            this->_json = json::parse(starts_with(this->get("content-type"), "application/json") ? this->text() : "");

        return this->_json;
    }

    std::vector<std::string> header::list() const {
        return this->_list;
    }

    std::string request::message() const {
        return this->_message;
    }

    status_code response_t::status() const {
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

    // Non-Member Fields

    std::atomic<size_t> _max_redirects = 20;
    std::atomic<size_t> _timeout = 30;

    // Non-Member Functions

    response _parse_response(const std::string data) {
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

        int          cl = headers["content-length"];
        std::string  text;
        trailer::map trailers;

        if (cl == INT_MIN) {
            headers.erase("content-length");
            
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

                    chunks.push_back(str.substr(0, std::min(size, (int) str.length())));
                }

                text = join(chunks, "\r\n");

                // Parse trailers
                while (getline(iss, str)) {
                    std::vector<std::string> trailer = split(str, ":");

                    if (trailer.size() == 1)
                        break;

                    trailers[tolowerstr(trailer[0])] = trim(str.substr(trailer[0].length() + 1));
                }   
            } else if (headers["transfer-encoding"].str().empty())
                headers.erase("transfer-encoding");
        } else
            text = oss.str().substr(0, std::min(cl, (int) oss.str().length()));

        if (status < 200 || status >= 400)
            throw fetch::error(static_cast<status_code>(status), status_text, text, headers, trailers);

        return response(static_cast<status_code>(status), status_text, headers, text, trailers);
    }

    response _request(header::map& headers, const std::string url, const std::string method, const std::string body, const size_t redirects, const size_t max_redirects) {
        class request request(headers, url, method, body);
        class url     url_obj = request.url();

        if (url_obj.protocol() == "https")
            throw fetch::error(UNKNOWN_ERROR, strstatus(UNKNOWN_ERROR), "TLS is not supported");

        try {
            std::vector<class host> hosts = dns::lookup(url_obj);

            try {
                tcp_client*       client = new tcp_client(hosts[0].ip(), url_obj.port());
                std::atomic<bool> recved = false;

                try {
                    client->send(request.message());

                    // Begin - Listen for timeout
                    std::thread([&recved, &client]() {
                        size_t timeout = get_timeout();

                        for (size_t i = 0; i < timeout && !recved.load(); i++)
                            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                        if (recved.load())
                            return;

                        recved.store(true);

                        // Sever connection
                        client->close();
                    }).detach();
                    // End - Listen for timeout
                    
                    std::string response = client->recv();

                    logger::debug(response);

                    recved.store(true);
                    client->close();

                    // Server disconnected
                    if (response.empty())
                        throw fetch::error(UNKNOWN_ERROR, strstatus(UNKNOWN_ERROR));

                    class response response_obj = _parse_response(response);

                    // Redirect
                    if (response_obj.status() >= 300 && response_obj.status() < 400) {
                        std::string location = response_obj.headers()["location"];

                        if (location.length()) {
                            if (redirects == max_redirects)
                                throw fetch::error(UNKNOWN_ERROR, strstatus(UNKNOWN_ERROR), "Maximum redirects");

                            logger::info("Redirecting to " + location);

                            return _request(headers, location, method, body, redirects + 1, max_redirects);
                        }
                    }

                    return response_obj;
                } catch (mysocket::error& e) {
                    if (recved.load())
                        throw fetch::error(UNKNOWN_ERROR, strstatus(UNKNOWN_ERROR), "Connection timed out");

                    client->close();

                    throw e;
                }
            } catch (mysocket::error& e) {
                throw fetch::error(UNKNOWN_ERROR, strstatus(UNKNOWN_ERROR), e.what());
            }
        } catch (dns::error& e) {
            throw fetch::error(UNKNOWN_ERROR, strstatus(UNKNOWN_ERROR), e.what());
        }
    }
    
    size_t get_max_redirects() {
        return _max_redirects.load();
    }

    size_t get_timeout() {
        return _timeout.load();
    }

    std::string http_version() {
        return "HTTP/1.1";
    }

    std::atomic<size_t>& max_redirects() {
        return _max_redirects;
    }

    response request(header::map& headers, const std::string url, const std::string method, const std::string body) {
        return _request(headers, url, method, body, 0, get_max_redirects());
    }

    void set_max_redirects(const size_t value) {
        return _max_redirects.store(value);
    }

    void set_timeout(const size_t value) {
        _timeout.store(value);
    }

    std::string strstatus(const status_code status) {
        switch (status) {
            case UNKNOWN_ERROR:
                return "Unknown error";
            case OK:
                return "OK";
            case NO_CONTENT:
                return "No Content";
            case FOUND:
                return "Found";
            case TEMPORARY_REDIRECT:
                return "Temporary Redirect";
            case PERMANENT_REDIRECT:
                return "Permanent Redirect";
            case BAD_REQUEST:
                return "Bad Request";
            case UNAUTHORIZED:
                return "Unauthorized";
            case NOT_FOUND:
                return "Not Found";
            case INTERNAL_SERVER_ERROR:
                return "Internal Server Error";
            default:
                break;
        }
        
        return "";
    }
}
