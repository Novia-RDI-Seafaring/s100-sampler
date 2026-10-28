# Limitations and known issues

`s100-sampler` is a research and integration client for testing S-124, S-125 and S-212 services. It is not an ECDIS or a navigation system, and must not be used for navigation. This page lists what it does not do and the known defects, so you can judge whether it fits your test.

## Scope

- **No chart display.** The commercial chart engine was removed; see [External requirements](EXTERNAL-REQUIREMENTS.md#chart-engine-licence-and-charts). Chart menus and settings left in the user interface do nothing.
- **No own-ship data.** The NMEA serial input was removed with the chart integration.
- **Tested on Windows only.** Linux builds are not yet tested.
- **Specification editions.** The parsers were written for the S-124, S-125 and S-212 editions used in 2024–2025, including S-124 edition 2.0.0. Other editions may parse only partly.
- **S-125 display.** Only AtoN with a status change (temporary change or discrepancy) are listed.

## Security

These matter if you connect to real services.

| Issue | Where | Effect |
| --- | --- | --- |
| TLS server verification can be turned off | `verify_tls` in `config.toml` | On by default. With `verify_tls = false` the client cannot detect an impostor server; use it only for test servers |
| Messages are not signed | `src/S212/VTSDataLayer.cpp` | SECOM signature fields carry placeholder values |
| Local backend profile has no TLS or authentication | `docs/backend/application-local.properties.example` | Use it only on `127.0.0.1` |

## Known defects

| Issue | Where | Workaround |
| --- | --- | --- |
| File extensions are case-sensitive | `S100DataLayer::listXMLFilesInFolder` | Rename `.GML`/`.XML` files to lower case |
| Service registries are hard-coded, and the national test registry was not reachable from a public network in October 2026 | `src/MSR/MSRMainModule.cpp` | Use the MCP Public Demonstrator, or edit the source to add or change a registry |
| The Windows package cannot send S-212 messages on other PCs: the schema check runs `xmllint` from a path fixed at build time, and the package does not include it | `XMLLINT_EXECUTABLE` in `CMakeLists.txt`, `VTSMessageWrapper::validateXML` | Build from source |
| The Windows package is not code-signed, so SmartScreen warns and managed PCs may block it | Release package | See [Troubleshooting](TROUBLESHOOTING.md#access-is-denied-when-starting-the-client-from-the-windows-package) |
| When opened, the MSR dialog lists all stored services, whichever registry is selected in the drop-down | `MSRMainModule::init()` | Click **Load Services** to show only the selected registry's services |
| The dialog for SECOM Get Summary and dataset subscriptions is disabled | `src/mainUI/MainWindow.cpp` | None |
| A background thread in the VTS dialog is detached and may outlive it | `src/mainUI/vtsdialog.cpp` | None |

## Testing

There are no automated tests and no continuous integration. The software was tested by hand during development and in the MaDaMe pilots of 2025, using the chart-enabled NoviaECS version. The release described here has been built and run on Windows, and the client has been checked against a locally running backend (October 2026). The full S-212 exchange with a VTS service has not been repeated with this release.

<!-- TODO: Update this section after the clean build and the reproduction of the paper's examples. Remove items as they are fixed. -->

## Reporting a problem

Open an issue on GitHub. Include the repository revision (`git rev-parse HEAD`), your operating system, compiler and Qt versions, the command you ran, and the first error message. For network problems, attach the relevant files from `build/netlogs/` after removing any private data.
