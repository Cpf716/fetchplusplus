//
//  fetch.h
//  fetch-json
//
//  Created by Corey Ferguson on 9/2/25.
//

#ifndef fetch_h
#define fetch_h

#include "json.h"
#include "socket.h"
#include <fstream>

namespace fetch {
    // Typedef

    struct header {
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
        int                      int_value();

        std::vector<std::string> list() const;

        std::string              str() const;

        // Typedef

        using map = std::map<std::string, header>;
    private:
        // Member Fields

        int                      _int;
        std::vector<std::string> _list;
        bool                     _parsed = false;
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
        virtual header       get(const std::string key) = 0;

        virtual header::map  headers() = 0;

        virtual size_t       status() const = 0;

        virtual std::string  status_text() const = 0;

        virtual std::string  text() const = 0;

        virtual trailer::map trailers() = 0;
    };

    struct error: public std::exception, public response_t {
        // Constructors

        error(
            const size_t      status,
            const std::string status_text,
            const std::string text = "",
            header::map       headers = {},
            trailer::map      trailers = {}
        );

        // Member Functions

        /**
         * Return response header
         */
        header       get(const std::string key);

        header::map  headers();

        size_t       status() const;

        std::string  status_text() const;

        std::string  text() const;

        trailer::map trailers();

        const char*  what() const throw();
    private:
        // Member Fields

        header::map  _headers;
        size_t       _status;
        std::string  _status_text;
        std::string  _text;
        trailer::map _trailers;
    };

    struct response: public response_t {
        // Constructors

        response(
            const size_t      status,
            const std::string status_text,
            header::map       headers,
            const std::string text,
            trailer::map      trailers
        );

        ~response();

        // Member Functions

        /**
         * Return response header
         */
        header        get(const std::string key);

        header::map   headers();

        json::object* json();

        size_t        status() const;

        std::string   status_text() const;

        std::string   text() const;

        trailer::map  trailers();
    private:
        // Member Fields
        
        header::map   _headers;
        json::object* _json = NULL;
        size_t        _status;
        std::string   _status_text;
        std::string   _text;
        trailer::map  _trailers;
    };

    // Non-Member Functions
    
    response request(header::map& headers, const std::string url, const std::string method = "GET", const std::string body = "");

    /**
     * Returns request timeout
     */
    size_t&  timeout();
}

#endif /* fetch_h */
