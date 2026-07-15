//
//  logger.cpp
//  fetch-json
//
//  Created by Corey Ferguson on 2/12/26.
//

#include "logger.h"

// Constructors

logger::logger() : logger(LOG_SOME) { }

logger::logger(const logging level) {
    this->_level = level;
}

// Non-Member Functions

void _log(const std::string message) {
    std::cout << message << std::endl;
}

// Member Functions

void logger::error(const std::string message) {
    _log("Error - " + message);
}

logging& logger::level() {
    return this->_level;
}

void logger::log(const enum logging logging, const std::string message) {
    if (this->level() >= logging)
        _log(message);
}

void logger::more(const std::string message) {
    this->log(LOG_MORE, message);
}

void logger::some(const std::string message) {
    this->log(LOG_SOME, message);
}
