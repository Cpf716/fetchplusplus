//
//  file.cpp
//  fetch-json
//
//  Created by Corey Ferguson on 7/14/26.
//

#include "file.h"

namespace file {
    // Constructors

    error::error(const std::string what) {
        this->_what = what;
    }

    // Member Functions

    const char* error::what() const throw() {
        return this->_what.c_str();
    }

    // Non-Member Functions

    std::string read(const std::string filename) {
        std::ifstream file;
        
        file.open(filename);
        
        if (!file.is_open())
            throw error(std::string(strerror(errno)));
        
        std::ostringstream ss;
        
        ss << file.rdbuf();
        
        return ss.str();
    }

    void write(const std::string filename, const std::string data) {
        std::ofstream file;
        
        file.open(filename);
        
        if (!file.is_open())
            throw error(std::string(strerror(errno)));
        
        file << data;
        
        if (file.fail())
            throw error(std::string(strerror(errno)));
    }
}