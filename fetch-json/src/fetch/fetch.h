//
//  fetch.h
//  fetch-json
//
//  Created by Corey Ferguson on 9/2/25.
//

#ifndef fetch_h
#define fetch_h

#include "dns.h"
#include "json.h"
#include "logger.h"
#include "socket.h"
#include <fstream>

using namespace dns;
using namespace json;
using namespace mysocket;

namespace fetch {
    // Typedef

    enum status_code {
        UNKNOWN_ERROR = 0,
        OK = 200,
        NO_CONTENT = 204,
        FOUND = 302,
        TEMPORARY_REDIRECT = 307,
        PERMANENT_REDIRECT = 308,
        BAD_REQUEST = 400,
        UNAUTHORIZED = 401,
        NOT_FOUND = 404,
        INTERNAL_SERVER_ERROR = 500,
    };

    struct header {
        // Typedef

        using map = std::map<std::string, header>;

        // Constructors

        header();

        header(const char* value);

        header(const int value);

        header(const std::string value);

        header(const std::vector<std::string> value);

        // Operators

        operator                 int();

        operator                 std::string();

        operator                 std::vector<std::string>();

        int                      operator=(const int value);

        std::string              operator=(const std::string value);

        std::vector<std::string> operator=(const std::vector<std::string> value);

        bool                     operator==(const char* value);

        bool                     operator==(const int value);

        bool                     operator==(const std::string value);

        bool                     operator==(const header value);

        bool                     operator!=(const char* value);

        bool                     operator!=(const int value);

        bool                     operator!=(const std::string value);

        bool                     operator!=(const header value);

        // Member Functions

        int                      int_value() const;

        std::vector<std::string> list() const;

        std::string              str() const;
    private:
        // Member Fields

        int                      _int;
        std::vector<std::string> _list;
        std::string              _str;

        // Member Functions

        int                      _set(const int value);

        std::string              _set(const std::string value);

        std::vector<std::string> _set(const std::vector<std::string> value);
    };

    struct trailer: public header {
        // Typedef

        using map = header::map;
    };

    struct response_t {
        // Member Functions

        /**
         * Return response header
         */
        header       get(const std::string key);

        header::map  headers();

        status_code  status() const;

        std::string  status_text() const;

        std::string  text() const;

        trailer::map trailers();
    protected:
        // Member Fields
        
        header::map   _headers;        
        status_code   _status;
        std::string   _status_text;
        std::string   _text;
        trailer::map  _trailers;
    };

    class response: public response_t {
        // Member Fields

        json::object* _json = NULL;

        // Constructors

        response(const std::string data);

        // Non-Member Functions

        friend response _request(header::map& headers, const std::string url, const std::string method, const std::string body, const size_t redirects, const size_t max_redirects);
    public:
        // Constructors

        response();

        response(const status_code status, const std::string status_text, const std::string text);

        response(const status_code status, const std::string status_text, header::map headers = {}, const std::string text = "", trailer::map trailers = {});

        ~response();

        // Member Functions

        json::object* json();
    };

    class request {
        // Constructors

        request(header::map& headers, const std::string url, const std::string method, const std::string body);

        // Member Fields

        std::string _message;
        url         _url;

        // Non-Member Functions

        friend response _request(header::map& headers, const std::string url, const std::string method, const std::string body, const size_t redirects, const size_t max_redirects);

public:
        // Member Functions

        std::string message() const;

        url         url();
    };

    struct error: public std::exception, public response_t {
        // Constructors

        error(const status_code status, const std::string status_text, const std::string text = "", header::map headers = {}, trailer::map trailers = {});

        // Member Functions

        const char* what() const throw();
    private:

        // Non-Member Functions

        friend response _request(header::map& headers, const std::string url, const std::string method, const std::string body, const size_t redirects, const size_t max_redirects);
    };

    // Non-Member Functions

    size_t               get_max_redirects();

    size_t               get_timeout();

    std::string          http_version();

    std::atomic<size_t>& max_redirects();

    response             request(header::map& headers, const std::string url, const std::string method = "GET", const std::string body = "");

    void                 set_max_redirects(const size_t value);

    void                 set_timeout(const size_t value);

    std::string          strstatus(const status_code status);
}

#endif /* fetch_h */
