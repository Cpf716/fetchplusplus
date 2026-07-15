//
//  json.h
//  fetch-json
//
//  Created by Corey Ferguson on 9/2/25.
//

#ifndef json_h
#define json_h

#include "util.h"
#include <cassert>

namespace json {
    // Typedef
    
    struct error: public std::exception {
        // Constructors

        error(const std::string what);

        // Member Fields

        const char* what() const throw();
    private:
        // Member Fields
        
        std::string _what;
    };

    struct object {
        // Typedef
        
        enum type { JSON_ARRAY_TYPE, JSON_OBJECT_TYPE, JSON_PRIMITIVE_TYPE };
        
        // Constructors
        
        object();
        
        object(const std::string key);

        /**
         * options:
         * - (k)ey
         * - (t)ext
         * - (v)alue
         */
        object(std::map<std::string, std::string> options);

        object(const enum type type);
        
        object(const std::string key, const enum type type);
        
        object(const std::string key, const std::string value);

        object(const std::vector<object*> values, const enum type type = JSON_OBJECT_TYPE);

        ~object();

        // Member Functions

        /**
         * Set undefined
         */
        void         erase();

        /**
         * Set item undefined
         */
        void         erase(const size_t index);
        
        /**
         * Delete property
         */
        void         erase(const std::string key);

        /**
         * Return property if it exists, otherwise return NULL
         */
        object*      get(std::string key);

        std::string  key();

        bool         null();

        void         nullify();

        double       number();
                
        /**
         * Delete undefined properties
         */
        object*      sanitize();

        /**
         * Set array item or object property and return it
         */
        object*      set(object* value);
        
        /**
         * Return array size
         */
        size_t       size();

        std::string  string();

        enum type&   type();
        
        bool         undefined();

        std::string& value();

        // Non-Member Functions

        friend void                 __pretty_print(std::ostringstream& ss, object* value, const int index);

        friend void                 _pretty_print(std::ostringstream& ss, object* value, const int index);

        friend void                 _stringify(std::ostringstream& ss, object* value, const int index);

        friend object*              assign(object* target, object* source);

        friend std::string          stringify(object* value, bool pretty);

        friend std::vector<object*> values(object* value);
    protected:
        // Member Fields

        std::string          _key;
        std::vector<object*> _values;
    private:
        // Member Fields
        
        std::unordered_map<std::string, size_t> _key_map;
        enum type                               _type = JSON_PRIMITIVE_TYPE;
        std::string                             _value;
        
        // Member Functions

        void    _erase(const std::string key);

        /**
         * Perform binary search and return the relative index of key
         */
        int     _find(const std::string key);

        /**
         * Build key map
         */
        void    _map_keys();

        /**
         * Parse JSON string to object
         */
        void    _parse(const std::string text);
        
        object* _parse(object* target, std::vector<std::string>& source, const int start, const int end);
    };

    class array: public object {
        // Member Functions

        json::array* _splice(const int start, const int delete_count, const std::vector<object*> values);
    public:
        // Typedef

        struct iterator {
            // Typdef

            friend json::array;

            // Operators

            object*   operator*() const;

            // object*   operator->() const;

            iterator& operator+(int value);

            iterator& operator++();

            iterator& operator++(int);

            iterator& operator-(int value);

            iterator& operator--();

            iterator& operator--(int);

            bool      operator==(const iterator& value) const;

            bool      operator!=(const iterator& value) const;
        private:
            // Member Fields

            int                  _index = 0;
            size_t               _size;
            std::vector<object*> _values;

            // Constructors

            iterator(const size_t size, std::vector<object*> values);
        };

        // Constructors
        
        array();
        
        array(const std::string key);

        array(const size_t size);

        array(const std::vector<object*> values);

        // Member Functions

        object*      at(const int index);

        iterator     begin();

        json::array* concat(std::vector<object*> values);

        iterator     end();

        /**
         * Return property if it exists, otherwise return NULL
         */
        object*      get(std::string key);

        /**
         * Return value at index if it exists, otherwise return NULL
         */
        object*      get(const size_t index);

        /**
         * Set array item or object property and return it
         */
        object*      set(object* value);

        /**
         * Shorthand for array items
         */
        object*      set(const size_t index, object* value);

        json::array* slice(const int start);

        json::array* slice(int start, int end);

        json::array* splice(int start);

        json::array* splice(const int start, const int delete_count);
        
        json::array* splice(int start, int delete_count, const std::vector<object*> values);
    };

    // Non-Member Functions

    object*                                      assign(object* target, object* source);

    std::vector<std::pair<std::string, object*>> entries(object* value);

    std::vector<std::string>                     keys(object* value);

    std::string                                  null();

    object*                                      parse(const std::string text);

    std::string                                  stringify(object* value, bool pretty = false);

    std::string                                  typestr(object* value);

    std::string                                  undefined();

    std::vector<object*>                         values(object* value);
}

#endif /* json_h */
