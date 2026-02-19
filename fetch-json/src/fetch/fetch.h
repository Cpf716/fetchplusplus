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

    struct abstract_response {
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

        // Constructors

        abstract_response();
    };

    class error: public std::exception, public abstract_response {
        // Member Fields

        std::string _what;
    public:
        // Constructors

        error(const status_code status, const std::string status_text, const std::string text = "", header::map headers = {}, trailer::map trailers = {});

        // Member Functions

        const char* what() const throw();
    };

    class response: public abstract_response {
        // Member Fields

        json::object* _json = NULL;

        // Constructors

        response(const std::string data);
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
        // Typedef
        
        friend class http_client;

        // Constructors

        request(header::map& headers, const std::string url, const std::string method, const std::string body);

        // Member Fields

        std::string _message;
        url         _url;
public:
        // Member Functions

        std::string message() const;

        url         url();
    };

    class http_client {
        // Typedef

        struct pool {
            // Typedef
    
            class connection {  
                // Member Fields

                size_t      _max = INT_MAX;
                size_t      _number = 0;
                bool        _released = false;
                size_t      _timeout = 0;
                tcp_client* _value = NULL;
            public:
                // Typedef
                
                using map = std::map<std::string, connection>;

                // Constructors

                connection();

                connection(tcp_client* value);

                // Member Functions

                size_t&     max();

                size_t&     number();

                bool&       released();

                size_t&     timeout();

                tcp_client* value() const;
            };

            friend class http_client;
            
            // Constructors
            
            pool(const class logger logger);

            // Member Functions

            size_t      close(const std::string host);
            
            void        configure(const std::string host, std::function<void(connection*)> cb);

            tcp_client* get_connection(const std::string host, class url url);
        
            void        release(const std::string host, class url url);
        private:
            // Member Fields

            connection::map _connections;
            logger          _logger;
            std::mutex      _mutex;
            
            // Member Functions
            
            size_t      _close(const std::string host);
        };
        
        // Member Fields

        logger                   _logger;
        int                      _max_redirects = 20;
        pool                     _pool = pool(_logger);
        std::atomic<bool>        _recved = true;
        std::vector<std::thread> _threads;
        int                      _timeout = 30;

        // Member Functions
        
        response _request(header::map& headers, const std::string url, const std::string method, const std::string body, const size_t redirects, const size_t max_redirects);
    public:

        // Constructors

        http_client();

        http_client(class logger logger);

        ~http_client();

        // Member Functions
        
        response get(header::map& headers, const std::string url);
        
        response head(header::map& headers, const std::string url, const std::string body = "");
        
        int&     max_redirects();
        
        response options(header::map& headers, const std::string url);
        
        response post(header::map& headers, const std::string url, const std::string body);
        
        response put(header::map& headers, const std::string url, const std::string body);

        response request(header::map& headers, const std::string url, const std::string method = "get", const std::string body = "");
        
        int&     timeout();
    };

    // Non-Member Functions

    // auto        _lock(std::mutex& mtx, auto cb);

    // response    _parse_response(const std::string data);

    std::string http_version();

    std::string strstatus(const status_code status);
}

#endif /* fetch_h */
