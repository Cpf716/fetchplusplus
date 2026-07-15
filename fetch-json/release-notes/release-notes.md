# Transport Layer Security (TLS)

Fetch++ now features transport layer security, allowing you interact with almost any resource on the internet. The powerful and robust HTTP client features strong encryption thanks to <i>Mbed TLS</i> as well as connection pooling, asynchronous requests, and native JSON compatibility in a modern and intuitive API.

Please see `main.cpp` for sample code.

## Change Log
* Implement TLS
* Handle multi-packet responses
* Reenable IPv6 DNS lookup
* Request HTTP/1.1 connections
* Disconnect immediately when no content-length header is provided
* Optimize JSON performance by replacing BSTs with hash tables and serializing with one continuous stream
* Implement verbose JSON parser errors
* Implement JSON pretty print
* Fix a bug where successive requests may trigger premature timeouts
* Fix a bug where pooled connections may close while in use, triggering segfaults
* Simplify test server

### Addendum
* Fix a bug where pooled connections are delaying program exit
* Fix a bug where large responses with content-length header may time out

### Limitations
* Transfer-encoding chunk size lines must not cross packet boundaries