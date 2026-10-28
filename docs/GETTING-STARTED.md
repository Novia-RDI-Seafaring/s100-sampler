# Getting started

This guide takes you from a fresh clone to a running client that shows S-124 navigational warnings from local files. That first result needs no server, certificate, or chart data. Later sections add live services and the S-212 VTS exchange.

`s100-sampler` has two parts:

| Part | Folder | Needed for |
| --- | --- | --- |
| Desktop client (C++17, Qt 5) | repository root | Everything |
| Companion backend (Java 17, Spring Boot) | `secom/` | S-212 VTS messages only |

The client has no chart display. The chart engine used in the original NoviaECS system is commercial and was removed; see [External requirements](EXTERNAL-REQUIREMENTS.md).

## What works with what you have

| You have | You can |
| --- | --- |
| The built client only | Load S-124 and S-125 files from disk and browse them in the NW and AtoN dialogs |
| + an MCP client certificate | Discover services in a Maritime Service Registry and download S-124/S-125 datasets from them |
| + the backend, reachable by a VTS service, and access to that service | Send S-212 traffic-clearance requests and receive replies and acknowledgements |

## Without building: the Windows package

Each release on GitHub has a Windows package, `s100-sampler-<version>-win64.zip`, under **Releases**. It contains the client and every DLL it needs, but not the backend.

1. Download the zip and unpack it into a folder of your own, for example `C:\Users\<you>\s100-sampler`.
2. Start the client from its `bin` folder, which must be the working directory:

   ```powershell
   Set-Location C:\Users\<you>\s100-sampler\s100-sampler-1.0.0-win64\bin
   .\s100-sampler.exe
   ```

3. The client creates `data\NW`, `data\AtoN` and `data\DB` in the unpacked folder, next to `config.toml`. Continue with [step 3](#3-first-result-s-124-warnings-from-files), using that `data` folder.

The program is not code-signed, so Windows SmartScreen may show **Windows protected your PC**; choose **More info → Run anyway**. On a PC managed by an organisation, a security policy may block it instead; see [Troubleshooting](TROUBLESHOOTING.md#access-is-denied-when-starting-the-client-from-the-windows-package).

The package cannot send S-212 messages on your PC: their schema check runs `xmllint` from the folder where the package was built, and the package does not include it. Build from source for S-212; see [Limitations](LIMITATIONS.md).

## 1. Build the client on Windows

Windows 11 x64 is the only tested platform. Linux is not yet tested.

**Tested environment**

| Requirement | Clean build, 26 October 2026 | Earlier build |
| --- | --- | --- |
| Windows | 11 x64 (build 26200) | 11 x64 |
| Visual Studio, "Desktop development with C++" workload | 2026 Insiders 18.12, MSVC 14.52.37025 | 2026 Community, MSVC 14.51.36231 |
| Windows SDK | 10.0.28000.0 | 10.0.28000.0 |
| CMake | 4.3.1 (bundled with Visual Studio) | 4.5.0-rc1 |
| vcpkg | Bundled with Visual Studio | Bundled with Visual Studio |

The project needs CMake 3.21 or later, but the Visual Studio 2026 generator needs a CMake at least as new as the one bundled with Visual Studio 2026. Any recent Git works.

The dependency manifest `vcpkg.json` pins the vcpkg baseline, so vcpkg installs the same package versions on every machine: Qt 5.15.19, OpenSSL 3.6.3, curl 8.21.0, libxml2 2.15.3, SQLiteCpp 3.3.3, pugixml 1.16, toml++ 3.4.0 and nlohmann/json 3.12.0. On the clean build, vcpkg built 37 packages in 41 minutes.

Clone the repository:

```powershell
git clone https://github.com/Novia-RDI-Seafaring/s100-sampler.git
Set-Location .\s100-sampler
```

Open **Developer PowerShell for VS** from the Start menu and go to the checkout. This terminal uses the CMake and vcpkg that come with Visual Studio. A separately installed CMake that is older than your Visual Studio does not know its generator and stops with `Could not create named generator`.

Check what the terminal will use:

```powershell
cmake --version
$env:VCPKG_ROOT
```

Configure. Use the generator for your Visual Studio version: `"Visual Studio 18 2026"` or `"Visual Studio 17 2022"`.

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
```

If `$env:VCPKG_ROOT` is empty, give the full path instead, for example `C:/Program Files/Microsoft Visual Studio/18/Community/VC/vcpkg/scripts/buildsystems/vcpkg.cmake`. Replace `Community` with your edition (`Professional`, `Enterprise` or `Insiders`).

If `cmake --version` still reports an older CMake, a separately installed CMake comes first on your `PATH`. Call the one bundled with Visual Studio by its full path instead, in any terminal (Command Prompt syntax; adjust `18\Insiders` to your version and edition):

```bat
set CMAKE="C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
%CMAKE% -S . -B build -G "Visual Studio 18 2026" -A x64 -DCMAKE_TOOLCHAIN_FILE="C:/Program Files/Microsoft Visual Studio/18/Insiders/VC/vcpkg/scripts/buildsystems/vcpkg.cmake"
%CMAKE% --build build --config Release --parallel 4
```

If configuration fails, fix the error and delete the `build` folder before trying again.

vcpkg installs the dependencies during this step. The first run builds Qt and can take an hour or more. Wait for `Generating done`.

Build:

```powershell
cmake --build build --config Release --parallel 4
```

The result is `build\Release\s100-sampler.exe`, with the required DLLs copied next to it.

To make the Windows package described above, run CPack from `build` after a Release build. It uses `cpack.exe` from the same folder as the CMake you used:

```powershell
Set-Location .\build
cpack -C Release
```

The result is `build\s100-sampler-<version>-win64.zip`.

Windows' 260-character path limit breaks the Qt build when the checkout is in a deep folder. Either clone into a short folder such as `C:\src`, or add `-DVCPKG_INSTALL_OPTIONS=--x-buildtrees-root=C:/vcpkg-bt` to the configure command so that vcpkg builds in a short folder. See [Troubleshooting](TROUBLESHOOTING.md).

## 2. Run the client

The client finds `config.toml`, `resources/` and `data/` relative to its **working directory**, which must be `build`:

```powershell
Set-Location .\build
& .\Release\s100-sampler.exe
```

On first start the client creates `data/NW`, `data/AtoN` and `data/DB` in the repository root. The main window opens with an empty area where the chart used to be, and a toolbar with **NW**, **AtoN** and **VTS** buttons.

Close it with **Menu → Close ECS**.

## 3. First result: S-124 warnings from files

1. Get S-124 datasets. The Canadian Coast Guard publishes S-124 edition 2.0.0 trial data at <https://nis.ccg-gcc.gc.ca/public/rest/messages/en/s124-trial>; any valid S-124 GML file works.
2. Close the client.
3. Copy the files into `data/NW/`. The client reads only files ending in lower-case `.xml` or `.gml`, so rename files such as `124CA01M_1286_26.GML` to `.gml`.
4. Start the client from `build` as in step 2.
5. Click **NW**. The dialog lists each warning by general area and publication time. Expand an entry to see the warning text; use the search field to filter.

At start-up the client parses every file in `data/NW` and records new ones in `data/DB/NW.db`. To reload the folder while the client is running, open the NW dialog and click **Manual NW update** with no S-124 service selected.

S-125 files work the same way through `data/AtoN/` and the **AtoN** button. Only AtoN that carry a status change are listed.

## 4. Live S-124 and S-125 services

Live data comes from SECOM services found through a Maritime Service Registry (MSR). You need a client certificate and key from the Maritime Connectivity Platform (MCP); see [External requirements](EXTERNAL-REQUIREMENTS.md#mcp-certificates).

1. Save the certificate and key as PEM files, for example in `certs/` at the repository root.
2. Set their paths in `config.toml`, relative to `build`:

   ```toml
   [network]
   certpath = "../certs/client_cert.pem"
   keypath  = "../certs/client_key.pem"
   ```

3. Start the client and open **Menu → MSR Menu**. Search a registry, then activate the S-124 or S-125 service instances you want. Activation assigns each instance to the matching module.
4. Open **NW** or **AtoN** and click **Manual NW update** or **Manual AtoN update**. The client downloads the datasets, saves them in `data/NW` or `data/AtoN`, and parses them.

The registry addresses are currently set in `src/MSR/MSRMainModule.cpp`, not in `config.toml`.

## 5. S-212 VTS messages

S-212 is two-way: the VTS service delivers its replies by uploading them to an endpoint on the ship side. That endpoint is the companion backend in `secom/`.

1. Build and start the backend: [SECOM backend](SECOM-BACKEND.md).
2. Point the client at it in `config.toml`:

   ```toml
   [network]
   api_endpoint = "http://127.0.0.1:8443"
   ```

3. Set your vessel identifier under `[ship] vessel_id`, and the VTS locations your service accepts under `[locations]`. See [Configuration](CONFIGURATION.md).
4. Activate an S-212 service instance in the MSR menu. The **VTS** button becomes active.
5. In the VTS dialog, click **+ NEW**, choose a location and send the request.

The client then polls the backend every 2.5 s for replies. For a VTS service to reach the backend, the backend must be reachable from that service over HTTPS, not only on `127.0.0.1`. The local profile in the backend guide is for testing the client–backend connection.

Messages are checked against an S-212 schema with `xmllint` before sending, and a message that fails the check is not sent. The schema is not included, so you must add it before you can send; see [External requirements](EXTERNAL-REQUIREMENTS.md#s-212-schema).

## Troubleshooting

The most common problems on a first set-up:

| Symptom | Fix |
| --- | --- |
| `Could not create named generator Visual Studio 18 2026` | Use the CMake that ships with Visual Studio (Developer PowerShell, or its full path) |
| `build is not a directory` | The configure step failed; fix its first error and delete `build` |
| Icons missing or `Parsing failed` at start-up | Start the client with `build` as the working directory |
| NW list is empty | Files in `data/NW`, lower-case `.xml`/`.gml`, client restarted |

See [Troubleshooting](TROUBLESHOOTING.md) for all known errors, how likely they are, and their fixes. Network requests and responses are logged in `build/netlogs/`.

## Next

- [External requirements](EXTERNAL-REQUIREMENTS.md): what is not in this repository and where to get it
- [SECOM backend](SECOM-BACKEND.md): build and run the companion backend
- [Configuration](CONFIGURATION.md): every `config.toml` setting
- [Architecture](ARCHITECTURE.md): how the modules and data flows fit together
- [Limitations and known issues](LIMITATIONS.md): what the release does not do, and workarounds
- [Troubleshooting](TROUBLESHOOTING.md): common errors and their fixes
