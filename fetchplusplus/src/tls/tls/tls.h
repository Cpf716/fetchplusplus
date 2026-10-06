//
//  tls.h
//  fetchplusplus
//
//  Created by Corey Ferguson on 7/10/26.
//

#ifndef tls_h
#define tls_h

#include "debug.h"
#include "dns.h"
#include "fpp_client.h"
#include "logger.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/x509_crt.h"
#include "socket.h"
#include <fcntl.h>

namespace tls {
    // Non-Member Fields

    static const char* ca_path = "/etc/ssl/cert.pem";

    // Typedef

    struct error: public fpp::error { 
        error(const std::string what);
    };

    class tls_client: public fpp::fpp_client {
        // Constructors

        ~tls_client();

        // Member Fields

        mbedtls_x509_crt         cacert;
        mbedtls_ssl_config       conf;
        mbedtls_ctr_drbg_context ctr_drbg;
        mbedtls_entropy_context  entropy;
        mbedtls_net_context      server_fd;
        mbedtls_ssl_context      ssl;
    public:
        // Constructors

        tls_client(const std::string hostname, const int port);

        tls_client(class logger* logger, const std::string hostname, const int port);

        // Member Functions

        void        close();

        std::string recv();

        int         send(const std::string message);
    };
}

#endif /* tls_h */
