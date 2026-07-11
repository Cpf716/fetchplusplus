//
//  util.h
//  fetch-json
//
//  Created by Corey Ferguson on 9/2/25.
//

#ifndef util_h
#define util_h

#include "logger.h"
#include <iostream>
#include <random>
#include <sstream>

// Non-Member Functions

int                      decimal(const std::string decimal);

/**
 * Decode double quotation-escaped string
 */
std::string              decode(const std::string string);

/**
 * Return string escaped by double quotations
 */
std::string              encode(const std::string string);

/**
 * Return true if value can be parsed into an integer, otherwise return false
 */
bool                     is_int(const std::string value);

/**
 * Return true if value can be parsed into a floating-point number, otherwise return false
 */
bool                     is_number(const std::string value);

bool                     is_pow(const size_t b, const size_t n);

bool                     is_string(const std::string value);

std::string              join(std::vector<std::string> values, std::string delimeter);

/**
 * Merge double quotation-escaped tokens
 */
void                     merge(std::vector<std::string>& values, const std::string delimiter = "");

/**
 * Return value parsed into an integer
 */
int                      parse_int(const std::string value);

/**
 * Return value parsed into a floating-point number
 */
double                   parse_number(const std::string value);

/**
 * Return the next power of two
 */
int                      pow2(const int b);

std::vector<std::string> split(const std::string string, const std::string delimeter);

void                     split(std::vector<std::string>& target, const std::string source, const std::string delimeter);

bool                     starts_with(const std::string text, const std::string pattern);

std::vector<std::string> tokens(const std::string string);

void                     tokens(std::vector<std::string>& target, const std::string source);

std::string              tolowerstr(std::string string);

std::string              toupperstr(std::string string);

/**
 * Return string trimmed of leading and trailing whitespace
 */
std::string              trim(const std::string string);

std::string              trim_end(const std::string string);

std::string              trim_start(const std::string string);

#endif /* util_h */
