# Fetch++

Please see `main.cpp` for sample code.

## Change Log
* Reimplement native JSON/XML requests/responses
* Efficiently serialize children of XML elements
* Inject _http_client_ logger dependency
* Fix a bug where an error response invariably closes the connection
* Fix a bug where HEAD requests w/ content-length response header hangs up

### Limitations
* Transfer-encoding chunk size lines must not cross packet boundaries