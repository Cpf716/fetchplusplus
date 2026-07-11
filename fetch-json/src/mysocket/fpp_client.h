#ifndef fpp_client_h
#define fpp_client_h

#define BUFF_LEN 65536 // 64 KB

#include <iostream>

namespace mysocket {
    struct fpp_client {
        virtual void        close() = 0;

        virtual std::string recv() = 0;

        virtual int         send(const std::string message) = 0;
    };

    struct fpp_error: public std::exception {
        // Member Fields

        const char* what() const throw();
    protected:
        // Member Fields

        std::string _what;
    };
}

#endif /* fpp_client_h */