# Fetch++

Please see `main.cpp` for sample code.

## Change Log
* Rewrite JSON serializer
* Optimize XML serializer
* Support list-type response headers
* Fix a bug where keep-alive max causes an error
* General optimizations
* Refactoring

### Limitations
* Transfer-encoding chunk size lines must not cross packet boundaries