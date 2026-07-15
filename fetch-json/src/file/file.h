//
//  file.h
//  fetch-json
//
//  Created by Corey Ferguson on 7/14/26.
//

#ifndef file_h
#define file_h

#include <fstream>
#include <iostream>
#include <sstream>

namespace file {
    // Typedef

    class error: public std::exception {
        // Member Fields

        std::string _what;
    public:
        // Constructors

        error(const std::string what);
    
        // Member Fields

        const char* what() const throw();
    };

    // Non-Member Functions

    std::string read(const std::string filename);

    void        write(const std::string filename, const std::string data);
}

#endif /* file_h */