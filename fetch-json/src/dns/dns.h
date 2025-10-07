//
//  dns.h
//  fetch-json
//
//  Created by Corey Ferguson on 10/6/25.
//

#ifndef dns_h
#define dns_h

#include "url.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>

namespace dns {
    // Typedef

    enum protocol { ipv4, ipv6 };

    class host {
        // Member Fields

        std::string   _ip;
        enum protocol _protocol;

        // Constructors

        host(const std::string ip, const enum protocol protocol);
    public:
        // Member Functions

        std::string   ip() const;

        enum protocol protocol() const;

        // Non-Member Functions

        friend std::vector<host> lookup(class url url);
    };

    class error: public std::exception {
        // Member Fields

        std::string _what;

        // Constructors

        error(const std::string what);
    public:
        // Member Fields

        const char* what() const throw();

        // Non-Member Functions

        friend std::vector<host> lookup(class url url);
    };

    // Non-Member Functions

    std::vector<host> lookup(class url url);
}

#endif /* dns_h */
