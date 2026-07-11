#include "tls.h"

namespace tls {
    // Non-Member Fields

    class logger _logger;

    // Constructors

    error::error(const std::string what) {
        this->_what = what;
    }

    tls_client::tls_client(const std::string hostname, const int port) {
        const char *pers = "mbedtls_client";
        
        mbedtls_net_init(&server_fd);
        mbedtls_ssl_init(&ssl);
        mbedtls_ssl_config_init(&conf);
        mbedtls_x509_crt_init(&cacert);
        mbedtls_entropy_init(&entropy);
        mbedtls_ctr_drbg_init(&ctr_drbg);

        // Initialize seed for the RNG
        if ((mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *) pers, strlen(pers)))) {
            // MBEDTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED
            throw tls::error("Entropy source failed");
        }

        // Read ca-bundle.crt
        if (mbedtls_x509_crt_parse_file(&cacert, ca_path().c_str())) {
            throw tls::error("Failed to read cert.pem");
        }

        // Configure SSL
        if (mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) {
            // MBEDTLS_ERR_XXX_ALLOC_FAILED
            throw tls::error("Memory allocation failed");
        }

        // Require SSL?
        mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);
        mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);

        mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

        // Initialize SSL session
        if (mbedtls_ssl_setup(&ssl, &conf)) {
            // PSA_ERROR_INSUFFICIENT_MEMORY
            throw tls::error("Memory allocation failed");
        }

        int errnum;

        // Set hostname
        if ((errnum = mbedtls_ssl_set_hostname(&ssl, hostname.c_str()))) {
            throw tls::error([errnum]() {
                switch (errnum) {
                    case MBEDTLS_ERR_SSL_ALLOC_FAILED:
                        return "Memory allocation failed";
                    case MBEDTLS_ERR_SSL_BAD_INPUT_DATA:
                        return "Bad hostname";
                    default:
                        return "";
                }
            }());
        }
        
        std::vector<class host> hosts;

        try {
            hosts = lookup(hostname);

            // Connect to the server
            if ((errnum = mbedtls_net_connect(&server_fd, hosts[0].ip().c_str(), std::to_string(port).c_str(), MBEDTLS_NET_PROTO_TCP))) {
                throw tls::error([errnum]() {
                    switch (errnum) {
                        case MBEDTLS_ERR_NET_SOCKET_FAILED:
                            return "Socket failed";
                        case MBEDTLS_ERR_NET_UNKNOWN_HOST:
                            return "Unknown host";
                        case MBEDTLS_ERR_NET_CONNECT_FAILED:
                            return "Connection failed";
                        default:
                            return "";
                    }
                }());
            }

            // Set socket to non-blocking
            int flags = fcntl(server_fd.fd, F_GETFL, 0);
            
            if (flags == -1 || fcntl(server_fd.fd, F_SETFL, flags | O_NONBLOCK) == -1) {
                throw tls::error("Setting socket options failed");
            }

            // Bind socket to the SSL context
            mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

            // Perform SSL handshake
            while ((errnum = mbedtls_ssl_handshake(&ssl))) {
                if (!(errnum == MBEDTLS_ERR_SSL_WANT_READ || errnum == MBEDTLS_ERR_SSL_WANT_WRITE)) {
                    // if (errnum == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED) { }
                    
                    throw tls::error("SSL handshake failed");
                }
            }

            _logger.extended("* Connected to " + hostname + "(" + hosts[0].ip() + ") port " + std::to_string(443));
        } catch (dns::error& e) {
            throw tls::error(e.what());
        }
    }

    tls_client::~tls_client() { }

    // Member Functions

    void tls_client::close() {
        mbedtls_net_free(&this->server_fd);
        mbedtls_x509_crt_free(&this->cacert);
        mbedtls_ssl_free(&this->ssl);
        mbedtls_ssl_config_free(&this->conf);
        mbedtls_ctr_drbg_free(&this->ctr_drbg);
        mbedtls_entropy_free(&this->entropy);
    }

    std::string tls_client::recv() {
        int           len,
                        total = 0;
        unsigned char buf[BUFF_LEN];

        // Listen for initial packet
        while ((len = mbedtls_ssl_read(&this->ssl, buf, sizeof(buf))) <= 0) {
            if (len == MBEDTLS_ERR_SSL_WANT_READ)
                continue;

            throw tls::error("Receive failed");
        }

        total += len;

        std::this_thread::sleep_for(std::chrono::milliseconds(2));

        // Listen until all packets are received
        while ((len = mbedtls_ssl_read(&this->ssl, buf + total, sizeof(buf) - total)) > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));

            total += len;
        }

        if (!(len == 0 || len == MBEDTLS_ERR_SSL_WANT_READ)) {
            throw tls::error("Receive failed");
        }

        return trim_end(std::string(reinterpret_cast<const char*>(buf)));
    }

    int tls_client::send(std::string message) {
        int len = 0,
             total = 0;

        // Repeat until all bytes have been sent
        while (total != message.length()) {
            len = mbedtls_ssl_write(&this->ssl, (const unsigned char *) message.substr(total).c_str(), message.length() - total);

            if (len <= 0)
                throw tls::error("Write failed");

            total += len;
        }

        return total;
    }

    // Non-Member Functions

    std::string ca_path() {
        return "/etc/ssl/cert.pem";
    }

    void set_logging(logging value) {
        _logger.logging() = value;
    }
}
