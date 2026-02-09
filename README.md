# s100-sampler

`s100-sampler` is a Qt-based ECS/VTS application for working with S-100 family data (including S-100, S-124, S-125, S-212).

The original version of the application included a licensed chart SDK (Nautilus SDK), which has been removed from this repository. As a result, parts of the source code and UI structure still reflect that original design. Despite this, the current version can be used to download S-100 data and inspect a subset of commonly used data fields.

The original version can be supplied separately upon request.

The project builds and runs on **Windows** and **Linux**.

---

## Dependencies

Core dependencies:

* C++17 compiler
* CMake 3.21
* Qt 5 (Widgets, Svg)
* libcurl
* OpenSSL
* SQLiteCpp
* pugixml
* nlohmann_json
* toml++

On **Windows**, dependencies are expected to be provided via **vcpkg**.
On **Linux**, either system packages or vcpkg can be used.

---

## Building

### Windows (Visual Studio + vcpkg)

1. Install vcpkg and required packages (example):

```
vcpkg install qt5-base qt5-svg curl openssl sqlitecpp pugixml nlohmann-json tomlplusplus cppcodec libxml2
```

2. Configure the project:

```
cmake -S . -B build ^
  -G "Visual Studio 17 2022" ^
  -A x64 ^
  -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
```

3. Build:

```
cmake --build build --config Release
```

The executable will be generated under the `build/` directory.

---

### Linux (example: GCC + Ninja)

Install dependencies via your package manager or vcpkg.

Example using system packages:

```
sudo pacman -S cmake ninja gcc qt5-base qt5-svg curl openssl pugixml nlohmann-json libxml2
```

Then configure and build:

```
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
```

---

## Configuration

The application uses a TOML configuration file named `config.toml`.

### Required user configuration

You **must** provide your own values for:

* API endpoint
* Client certificate path
* Client private key path

Example:

```
[network]
api_endpoint = "https://example.com:8443"

certpath = "../certs/client_cert.pem"
keypath  = "../certs/client_key.pem"
```

### Notes

* The API endpoint is required for **VTS functionality**.
* Certificates and private keys can be requested via the [MCP public demonstrator](https://management.maritimeconnectivity.net/).
* Paths are resolved relative to the executable working directory.
---

## VTS / SECOM Endpoint

The project directory contains a folder:

```
secom/
```

This directory contains an example backend implementation based on **SECOMLib** (Java / Spring Boot).

MSR provider endpoints are currently hard-coded; if different endpoints are required, this should be made configurable.

### Important clarification

* The connection between the ECS application and the endpoint is **not standard SECOM**.
* A **custom protocol** is used between the ECS and the endpoint.
* Therefore, **this specific endpoint implementation must be used** for VTS functionality to work correctly.
* A generic SECOM-compatible endpoint will **not** be sufficient.

The `secom` exists to provide the backend behavior expected by the ECS application.