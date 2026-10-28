# External requirements

This repository contains source code only. It contains no certificates, no service access, no maritime datasets, and no chart engine. This page lists everything you may need from elsewhere, what it is used for, and where to get it.

| Item | Needed for | Where to get it |
| --- | --- | --- |
| [Build tools and libraries](#build-tools-and-libraries) | Building the client | Visual Studio, CMake, vcpkg (installs the libraries) |
| [S-124 and S-125 datasets](#s-124-and-s-125-datasets) | Viewing warnings and AtoN offline | Public trial services, or a service provider |
| [MCP certificates](#mcp-certificates) | Downloading from live services | Maritime Connectivity Platform |
| [Service access](#service-access) | Live S-124, S-125 and S-212 data | The service provider |
| [S-212 schema](#s-212-schema) | Validating VTS messages before sending | The S-212 product specification |
| [Java 17, Maven and SECOMLib](#java-17-maven-and-secomlib) | The companion backend (S-212 only) | Public downloads; see the backend guide |
| [Chart engine, licence and charts](#chart-engine-licence-and-charts) | Chart display, which this release does not have | Commercial vendors; not needed to use this repository |

## Build tools and libraries

The client needs a C++17 compiler, CMake 3.21 or later, and vcpkg. vcpkg installs Qt 5, libcurl, OpenSSL, SQLiteCpp, pugixml, libxml2 (with `xmllint`), toml++, nlohmann/json and cppcodec from `vcpkg.json`. The tested Windows set-up is in [Getting started](GETTING-STARTED.md#1-build-the-client-on-windows).

## S-124 and S-125 datasets

No datasets are included. The datasets used during development came from service providers and may not be redistributed.

For testing without credentials:

- **S-124:** the Canadian Coast Guard publishes S-124 edition 2.0.0 trial data at <https://nis.ccg-gcc.gc.ca/public/rest/messages/en/s124-trial>. Check the terms on that page before you redistribute the files.
- **S-125:** no public trial source is known to us. Use datasets from a service provider, or retrieve them through a live service.

Place the files in `data/NW` (S-124) or `data/AtoN` (S-125) with the client closed. See [Getting started](GETTING-STARTED.md#3-first-result-s-124-warnings-from-files).

The parsers were written against the S-124 and S-125 editions in use in 2024–2025, including S-124 edition 2.0.0. Datasets in other editions may parse only partly.

<!-- TODO: State the exact S-124 and S-125 editions supported, after testing. -->

## MCP certificates

Downloading from live SECOM services, and the S-212 exchange, use mutual TLS with a client certificate issued through the Maritime Connectivity Platform (MCP). The MCP Consortium runs a public demonstrator where organisations can register and issue test certificates for vessels and services:

- MCP Management Portal (demonstrator): <https://management.maritimeconnectivity.net/>

Export the certificate and private key as PEM files, keep them outside the repository, and set `certpath` and `keypath` in `config.toml`. Never commit them.

A demonstrator certificate is accepted only by services that trust the demonstrator. Production services may require a certificate from another MCP instance.

## Service access

The client finds services through a Maritime Service Registry (MSR). Two registries are set in `src/MSR/MSRMainModule.cpp`: a national test registry and the MCP Public Demonstrator. Finding a service in a registry does not give you access to it.

- **S-124 and S-125:** the service provider decides which certificates it accepts. Ask the provider for access for your MCP identity.
- **S-212 VTS traffic clearance:** you need a VTS service that implements S-212 over SECOM and agrees to deliver replies to your backend. The provider must also accept your vessel identifier and the locations listed under `[locations]` in `config.toml`. The locations in the shipped `config.toml` are examples from the pilot area and work only with that provider.

S-212 also needs the companion backend to be reachable by the VTS service over HTTPS. A backend running only on `127.0.0.1` cannot receive replies from an external service.

## S-212 schema

Before sending, the client checks each S-212 message with:

```text
xmllint --noout --schema ../schemas/S212.xsd <message file>
```

`S212.xsd` is not included, because its distribution terms are not yet confirmed. Obtain the schema of the S-212 edition used by your VTS service from the S-212 product specification, and save it as `schemas/S212.xsd` in the repository root.

Without the schema, the check fails and the message is not sent. The client calls the `xmllint` that CMake found during configuration, so it need not be on `PATH`.

<!-- TODO: Add the official source of the S-212 schema (IALA). -->

## Java 17, Maven and SECOMLib

The backend needs Java 17, Maven, and SECOMLib 0.0.24 (`secom-core-jakarta` and `secom-springboot3`). SECOMLib is open source (<https://github.com/gla-rad/SECOMLib>). The published 0.0.24 lacks a data-product type the backend uses, so the [backend guide](SECOM-BACKEND.md) shows how to build it from source with a small patch included in this repository.

## Chart engine, licence and charts

**You do not need any of these to build or use this repository.** They are listed so you know what the original system used and why it is not here.

`s100-sampler` is the open-source service-client part of NoviaECS, a test S-100 Electronic Chart System developed at Novia University of Applied Sciences for the MaDaMe project. NoviaECS drew charts and S-124/S-125 overlays with a commercial chart SDK. The SDK, its licence and the chart-integration code were removed before publication:

| Removed | Why | Where it comes from |
| --- | --- | --- |
| Chart SDK (Teledyne SevenCs Nautilus) | Proprietary; may not be redistributed | The vendor, under a commercial licence |
| SDK runtime licence | Tied to the licence holder | Issued by the vendor with the SDK |
| Chart-integration source (chart widget, overlays, own-ship and NMEA handling) | Built on the SDK's interfaces | Not published |
| Electronic Navigational Charts (S-57/S-101) | Licensed by the chart producer | A hydrographic office or an authorised ENC distributor |
| Screenshots showing charts | Show licensed chart data | Not published |

Some class names, menus and `config.toml` keys (`[nautilus]`, `[charts]`, `[nmea]`) still refer to this integration, and the main window keeps the SDK calls as commented-out code. None of this has any effect in this release; see [Commented-out chart code](ARCHITECTURE.md#commented-out-chart-code).

The S-124 and S-125 modules still parse and store positions, geometry and status, so another chart engine could be connected to them. The main-window area where the chart widget was attached is the natural place to start.
