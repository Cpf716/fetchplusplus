# Fetch++

Thanks for checking out _fetch++_!

It combines three powerful SDKs, _fetch_, _json_, and _xml_, equipping C++ developers with a powerful native HTTP client for integration with most modern APIs.

# Setup
1. Open the project in VS Code and navigate to the integrated terminal
2. Ensure you have Node v18+ installed by running `node -v`
3. Run `npm install`
4. Open the project in Xcode
5. Click `Command + B` on the keyboard to build the project for the first time

## Build
1. Click `Command + ,` to open Settings, click _Locations_ in the sidebar, and click the arrow (->) under _Derived Data_
2. A new Finder window will open at Xcode’s _Derived Data_ directory; find your project's name and expand Build > Product > Debug
3. Launch Terminal and type `cd` + Space
4. Navigate back to Finder, click and drag the _Debug_ folder into Terminal, and click Enter
5. Click Command + N to open a new window or Command + T to open a new tab
6. Type `cd` + Space once more, drag your project's root directory into the new Terminal session, and click Enter
7. In the same Terminal session, execute the following command:<br><br>
_* You will have to execute the command every time you modify the C++ code_
```
xcodebuild -project fetch-json.xcodeproj -scheme fetch-json
```
8. Navigate back to Xcode and click Command + M to minimize the window

## Run
### Test Server

1. Navigate to VS Code's integrated terminal
2. Execute the following command:
```
node test-server/src/index.js
```

### Command-Line Application
1. Navigate to the first Terminal window
2. Execute the following command:
```
./fetch-json [-l | --log] # none, some, or more
```

The test server is for your convenience getting started, however Fetch++ is feature-rich, including robust transport layer security, so you can access almost any resource on the internet.

## Reference

By default, the _http_client_ executes synchronously. Request headers are mutated by the fetch SDK, therefore they must be passed by reference. The SDK natively accepts string, JSON, and XML arguments.

```
fetch::http_client http;
fetch::header::map headers;

std::string url = "http://localhost:8080/api/greeting";

try {
    http.options(headers, url);

    std::unique_ptr<json::object> body(
        (std::vector<json::object*>) {
            new json::object("firstName", escape("fetch++"))
        }
    );
    
    auto response = http.post(headers, url, body.get());
    
    std::unique_ptr<json::object> json_ptr(response.json());

    std::cout << unescape(json_ptr.get()->value()) << std::endl;
} catch (fetch::error& e) {
    throw e;
}
```
