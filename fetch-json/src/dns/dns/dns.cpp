//
//  dns.cpp
//  fetch-json
//
//  Created by Corey Ferguson on 10/6/25.
//

#include "dns.h"

namespace dns {
    // Constructors

    error::error(const std::string what) {
        this->_what = what;
    }

    host::host(const std::string ip, const enum protocol protocol) {
        this->_ip = ip;
        this->_protocol = protocol;
    }

    // Member Functions

    std::string host::ip() const {
        return this->_ip;
    }

    enum protocol host::protocol() const {
        return this->_protocol;
    }

    const char* error::what() const throw() {
        return this->_what.c_str();
    }

    // Non-Member Functions

    std::vector<host> lookup(class url url) {
        struct addrinfo hints;

        memset(&hints, 0, sizeof hints);

        hints.ai_family = AF_UNSPEC;     // IPv4 or IPv6
        
        // Remedy: error code 1003
        // hints.ai_family = AF_INET;     // IPv4
        
        hints.ai_socktype = SOCK_STREAM; // TCP

        int              s;
        struct addrinfo* result = NULL;

        if ((s = getaddrinfo(url.host().c_str(), NULL, &hints, &result)))
            throw dns::error(gai_strerror(s));

        std::vector<host> data;

        for (struct addrinfo* rp = result; rp != NULL; rp = rp->ai_next) {
            void*         addr = NULL;
            enum protocol protocol;

            if (rp->ai_family == AF_INET) {
                addr = &(((struct sockaddr_in *)rp->ai_addr)->sin_addr);
                protocol = ipv4;
            } else {
                addr = &(((struct sockaddr_in6 *)rp->ai_addr)->sin6_addr);
                protocol = ipv6;
            }

            char ip[INET6_ADDRSTRLEN];

            // Convert IP to string
            inet_ntop(rp->ai_family, addr, ip, sizeof ip);

            data.push_back(host(std::string(ip), protocol));
        }

        freeaddrinfo(result);

        return data;
    }
}
