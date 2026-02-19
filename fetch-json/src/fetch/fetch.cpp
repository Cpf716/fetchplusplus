//
//  fetch.cpp
//  fetch-json
//
//  Created by Corey Ferguson on 9/2/25.
//

#include "fetch.h"

namespace fetch {
    // Constructors

    abstract_response::abstract_response() { }

    http_client::pool::connection::connection() { }

    http_client::pool::connection::connection(tcp_client* value) {
        this->_value = value;
    }

    error::error(const status_code status, const std::string status_text, const std::string text, header::map headers, trailer::map trailers) {
        this->_status = status;
        this->_status_text = status_text;
        this->_text = text;
        this->_what = this->text().empty() ? this->status_text() : this->text();
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

    http_client::http_client() { }

    http_client::http_client(class logger logger) {
        this->_logger.logging() = logger.logging();
        this->_pool._logger.logging() = logger.logging();
    }

    http_client::pool::pool(const class logger logger) {
        this->_logger = logger;
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

            ss << this->url().target();
            
            if (this->url().params().size())
                ss << "?" << this->url().query();
            
            ss << " ";
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
            std::string host = this->url().fully_qualified_host();

            headers.erase("host");
            
            ss << "Host: " << host << "\r\n";
            
            auto it = headers.find("content-length");

            // Generate content-length, if required
            if (it == headers.end() && body.length())
                headers["content-length"] = (int) body.length();
            
            headers["user-agent"] = std::string("fetch-json/1.1");

            // Map request headers to hyphenated Pascal case
            for (const auto& [key, value]: headers) {
                std::vector<std::string> tokens = split(key, "-");
                
                for (size_t i = 0; i < tokens.size(); i++)
                    tokens[i] = std::string((char[]) { (char) toupper(tokens[i][0]), '\0' }) + tokens[i].substr(1);

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

    http_client::~http_client() {
        for (size_t i = 0; i < this->_threads.size(); i++)
            if (this->_threads[i].joinable())
                this->_threads[i].join();
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

    // Non-Member Functions

    auto _lock(std::mutex& mtx, auto cb) {
        mtx.lock();

        auto res = cb();

        mtx.unlock();

        return res;
    }

    response _parse_response(const std::string data) {
        // Begin - Parse response
        std::istringstream iss(data);
        std::string        line;

        getline(iss, line);

        std::vector<std::string> tokens = ::tokens(line);

        // Begin - Parse status and status text
        int status = parse_int(tokens[1]);

        // Merge status text
        for (size_t i = 3; i < tokens.size(); i++)
            tokens[2] += " " + tokens[i];

        std::string status_text = tokens[2];
        // End - Parse status and status text

        // Parse response headers
        header::map headers;

        while (getline(iss, line)) {
            std::vector<std::string> header = split(line, ":");
            
            // Empty line
            if (header.size() == 1)
                break;

            // Case-insensitive
            headers[tolowerstr(header[0])] = trim(line.substr(header[0].length() + 1));
        }

        // Parse response body
        std::ostringstream oss;

        while (getline(iss, line))
            oss << trim_end(line) << "\r\n";
        
        auto         it = headers.find("content-length");
        std::string  text;
        trailer::map trailers;

        if (it == headers.end()) {
            it = headers.find("transer-encoding");
            
            if (it != headers.end() && (* it).second.str() == "chunked") {
                // Parse response body
                iss.str(oss.str());
                iss.clear();

                std::vector<std::string> chunks;

                while (getline(iss, line)) {
                    int size = decimal(split(trim_end(line), ";")[0]);

                    if (size <= 0)
                        break;

                    getline(iss, line);

                    line = trim_end(line);

                    chunks.push_back(line.substr(0, std::min(size, (int) line.length())));
                }

                text = join(chunks, "\r\n");

                // Parse trailers
                while (getline(iss, line)) {
                    std::vector<std::string> trailer = split(line, ":");

                    if (trailer.size() == 1)
                        break;

                    trailers[tolowerstr(trailer[0])] = trim(line.substr(trailer[0].length() + 1));
                }
            }
        } else
            text = oss.str().substr(0, std::min((int) (* it).second, (int) oss.str().length()));

        if (status < 200 || status >= 400)
            throw fetch::error(static_cast<status_code>(status), status_text, text, headers, trailers);

        return response(static_cast<status_code>(status), status_text, headers, text, trailers);
    }

    std::string http_version() {
        return "HTTP/1.1";
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

    // Member Functions

    header abstract_response::get(const std::string key) {
        return this->_headers[key];
    }

    header::map abstract_response::headers() {
        return this->_headers;
    }

    status_code abstract_response::status() const {
        return this->_status;
    }

    std::string abstract_response::status_text() const {
        return this->_status_text;
    }

    std::string abstract_response::text() const {
        return this->_text;
    }

    trailer::map abstract_response::trailers() {
        return this->_trailers;
    }

    size_t& http_client::pool::connection::number() {
        return this->_number;
    }

    size_t& http_client::pool::connection::max() {
        return this->_max;
    }

    bool& http_client::pool::connection::released() {
        return this->_released;
    }

    size_t& http_client::pool::connection::timeout() {
        return this->_timeout;
    }

    tcp_client* http_client::pool::connection::value() const {
        return this->_value;
    }

    const char* error::what() const throw() {
        return this->_what.c_str();
    }

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

    int header::int_value() const {
        return this->_int;
    }

    std::vector<std::string> header::list() const {
        return this->_list;
    }

    std::string header::str() const {
        return this->_str;
    }

    response http_client::_request(header::map& headers, const std::string url, const std::string method, const std::string body, const size_t redirects, const size_t max_redirects) {
        class request request(headers, url, method, body);
        class url     url_obj = request.url();

        // TLS is not supported
        if (url_obj.protocol() == "https")
            throw fetch::error(UNKNOWN_ERROR, strstatus(UNKNOWN_ERROR), "Operation not permitted");
        
        this->_recved.store(false);

        try {
            std::chrono::time_point start = std::chrono::steady_clock::now();
            std::string             host = url_obj.fully_qualified_host();
            tcp_client*             client = this->_pool.get_connection(host, url_obj);
            
            this->_logger.extended(request.message());

            try {
                client->send(request.message());
                
                int timeout = this->timeout();

                // Begin - Listen for timeout
                this->_threads.push_back(std::thread([timeout, this, host]() {
                    for (size_t i = 0; i < timeout && !this->_recved.load(); i++)
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                    if (this->_recved.load())
                        return;

                    this->_recved.store(true);

                    // Sever connection
                    this->_pool.close(host);
                }));
                // End - Listen for timeout
                
                std::string response = client->recv();

                this->_recved.store(true);

                // Server disconnected
                if (response.empty())
                    throw fetch::error(UNKNOWN_ERROR, strstatus(UNKNOWN_ERROR));

                class response response_obj = _parse_response(response);

                // Preserve reference
                header::map response_headers = response_obj.headers();

                if (tolowerstr(response_headers["connection"]) == "keep-alive") {
                    header::map::iterator it = response_headers.find("keep-alive");
                    int                   timeout;
                    url::param::map       keep_alive;

                    if (it == response_headers.end())
                        timeout = 5;
                    else {
                        keep_alive = url::query_string(join((* it).second.list(), "&")).params();

                        // Implicitly converted from NAN to INT_MIN
                        timeout = floor(keep_alive["timeout"].number());

                        if (timeout < 1)
                            timeout = 5;
                    }
                    
                    this->_pool.configure(host, [timeout, &keep_alive](pool::connection* connection) {
                        connection->timeout() = timeout;

                        int max = floor(keep_alive["max"].number());

                        if (max >= 1)
                            connection->max() = max;
                    });
                    this->_pool.release(host, url_obj);
                } else
                    this->_pool.close(host);
                
                this->_logger.extended(response);
                
                // Redirect
                if (response_obj.status() >= 300 && response_obj.status() < 400) {
                    std::string location = response_obj.headers()["location"];

                    if (location.length()) {
                        if (redirects >= max_redirects)
                            throw fetch::error(UNKNOWN_ERROR, strstatus(UNKNOWN_ERROR), "Maximum redirects");
                        
                        // Preserve query string
                        if (request.url().params().size())
                            location += "?" + request.url().query();

                        this->_logger.info("Redirecting to " + location);

                        return _request(headers, location, method, body, redirects + 1, max_redirects);
                    }
                }

                this->_logger.extended(
                    std::to_string(std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count() * 1000) + " ms\n"
                );

                return response_obj;
            } catch (mysocket::error& e) {
                if (this->_recved.load())
                    throw fetch::error(UNKNOWN_ERROR, strstatus(UNKNOWN_ERROR), "Connection timed out");

                this->_pool.close(host);

                throw e;
            }
        } catch (mysocket::error& e) {
            throw fetch::error(UNKNOWN_ERROR, strstatus(UNKNOWN_ERROR), e.what());
        }
    }

    response http_client::get(header::map& headers, const std::string url) {
        return this->request(headers, url);
    }

    response http_client::head(header::map& headers, const std::string url, const std::string body) {
        return this->request(headers, url, "head", body);
    }

    int& http_client::max_redirects() {
        return this->_max_redirects;
    }

    response http_client::options(header::map& headers, const std::string url) {
        return this->request(headers, url, "options");
    }

    response http_client::post(header::map& headers, const std::string url, const std::string body) {
        return this->request(headers, url, "post", body);
    }

    response http_client::put(header::map& headers, const std::string url, const std::string body) {
        return this->request(headers, url, "put", body);
    }

    response http_client::request(header::map& headers, const std::string url, const std::string method, const std::string body) {
        return this->_request(headers, url, method, body, 0, this->max_redirects());
    }

    int& http_client::timeout() {
        return this->_timeout;
    }

    size_t http_client::pool::_close(const std::string host) {
        auto it = this->_connections.find(host);
        
        (* it).second.value()->close();
            
        this->_connections.erase(it);
        this->_logger.extended("");

        return 0;
    }

    size_t http_client::pool::close(const std::string host) {
        return _lock(this->_mutex, [this, host]() {
            return this->_close(host);
        });
    }

    void http_client::pool::configure(const std::string host, std::function<void(connection*)> cb) {
        _lock(this->_mutex, [cb, this, host]() {
            cb(&this->_connections[host]);
            
            return 0;
        });
    }

    tcp_client* http_client::pool::get_connection(const std::string host, class url url) {
        tcp_client* connection = _lock(this->_mutex, [this, host]() {
            return this->_connections[host].value();
        });

        if (connection == NULL) {
            try {
                std::vector<class host> hosts = lookup(url);

                try {
                    connection = new tcp_client(hosts[0].ip(), url.port());
                    
                    this->_logger.extended("* Connected to " + url.host() + "(" + hosts[0].ip() + ") port " + std::to_string(url.port()));

                    _lock(this->_mutex, [this, host, connection]() {
                        return this->_connections[host] = pool::connection(connection);
                    });
                } catch (mysocket::error& e) {
                    throw fetch::error(UNKNOWN_ERROR, strstatus(UNKNOWN_ERROR), e.what());
                }
            } catch (dns::error& e) {
                throw fetch::error(UNKNOWN_ERROR, strstatus(UNKNOWN_ERROR), e.what());
            }
        } else
            this->_logger.extended("* Getting pooled connection for host " + url.host());

        return connection;
    }

    void http_client::pool::release(const std::string host, class url url) {
        size_t number,
                timeout;

        if (_lock(this->_mutex, [this, host, &number, &timeout]() {
            pool::connection* connection = &this->_connections[host];

            if (connection->number() == connection->max())
                return this->_close(host);

            connection->released() = true;
            
            number = connection->number()++;
            
            return timeout = connection->timeout();
        }) == 0)
            return;
        
        std::string fully_qualified_host = url.host();

        std::thread([fully_qualified_host, timeout, this, host, number]() {
            this->_logger.extended("* Connection to host " + fully_qualified_host + " left intact");
            
            for (size_t i = 0; i < timeout; i++)
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            
            _lock(this->_mutex, [this, host, number]() {
                pool::connection* connection = &this->_connections[host];

                if (connection->number() == number + 1)
                    return this->_close(host);

                return (size_t) 0;
            });
        }).detach();
    }

    std::string request::message() const {
        return this->_message;
    }

    url request::url() {
        return this->_url;
    }

    json::object* response::json() {
        if (this->_json == NULL)
            this->_json = json::parse(starts_with(this->get("content-type"), "application/json") ? this->text() : "");

        return this->_json;
    }
}
