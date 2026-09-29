# Asynchronous Fetch
### Introducing asynchronous requests...
#### Often times during the application lifecycle, multiple resources need to be loaded simultanously. The latest version of fetch++ does just that with exceptional ease.</h4>

Asynchronous requests prove particularly effective for initial application loading as well as fetching multiple discrete but related resources among other use cases.

### Steps:

1. Initialize a <i>cpp17_latch</i> object with value <i>n</i> where <i>n</i> is the total number of requests to be made; the latch will block the main thread until <i>n</i> number of requests have returned
2. Count the latch down through your <i>request(...)</i> callbacks
3. Once the latch's internal counter has reached zero, `latch.wait()` returns

#### Drawbacks

The current implementation necessitates that all requests either return or time out before `latch.wait()` returns. Conservative timeouts can be leveraged to limit wait times. This <i>may</i> be improved in a future release. 

```
http_client http;

header::map h1, h2;

// Initialize latch with the number of requests
cpp17_latch latch(2);

auto cb = [&latch](class response response, fetch::error error) {
    latch.count_down();
};

try {
    http.get(h1, "http://localhost:8080/api/ping", cb);

    unique_ptr<object> body = make_unique<object>((vector<object*>) {
        new object("firstName", encode("$hy"))
    });

    http.post(h2, "http://localhost:8080/api/greeting", body, cb);
    
    // Block the main thread until all asynchronous requests have returned
    latch.wait();
} catch (fetch::error& e) {
    throw e;
}
```

## Error Handling
Errors are passed as Lambda arguments; however, if you still wish to throw an asynchronous error synchronously, you can do so with <i>exception_ptr</i>.

You may chose to throw an asynchronous error on the main thread if one or more required resources fail to load.

```
http_client http;

header::map h1, h2, h3;

// Initialize null exception_ptr
exception_ptr e;

// Initialize latch with the number of requests
cpp17_latch latch(3);

auto cb = [&e, &latch](class response response, fetch::error error) {
    try {
        // Throw error so it can be captured
        if (!response.ok())
            throw error;
    } catch (...) {
        e = current_exception();
    }

    latch.count_down();
};

try {
    http.get(h1, "http://localhost:8080/api/ping", cb);

    unique_ptr<object> body = make_unique<object>((vector<object*>) {
        new object("firstName", encode("$hy"))
    });

    http.post(h2, "http://localhost:8080/api/greeting", body, cb);

    // Fetch undefined path
    http.get(h3, "http://localhost:8080/api/no-path", cb)
    
    // Block the main thread until all asynchronous requests have returned
    latch.wait();

    // Throw error on main thread if one or more requests returned an error
    if (e) rethrow_exception(e);
} catch (fetch::error& e) {
    throw e;
}
```