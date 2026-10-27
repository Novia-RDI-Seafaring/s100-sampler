# Configuration

The client reads its settings from `config.toml` in the repository root. It opens the file as `../config.toml`, so the client must run with `build` as its working directory.

If the file is missing or not valid TOML, the client prints `Parsing failed` and starts with empty settings. If `certpath` or `keypath` is missing, it shows "Some network features disabled" at start-up.

Do not commit local values such as certificate paths or your vessel identifier. Keep a private copy of `config.toml`, or restore the tracked file before committing.

## Settings used by the release

### `[network]`

| Key | Required for | Example | Meaning |
| --- | --- | --- | --- |
| `api_endpoint` | S-212 | `"http://127.0.0.1:8443"` | Address of the [companion backend](SECOM-BACKEND.md). The client polls it for VTS replies and sends it to the VTS service as the callback address |
| `certpath` | Live services | `"../certs/client_cert.pem"` | MCP client certificate (PEM), relative to `build` |
| `keypath` | Live services | `"../certs/client_key.pem"` | Private key for that certificate (PEM), relative to `build` |
| `verify_tls` | All HTTPS requests | `true` | Verify the server's certificate and host name. Default `true`. Set `false` only for a test server whose certificate cannot be verified |
| `ca_bundle` | Optional | `"../certs/mcp-root-ca.pem"` | PEM file with extra trusted CA certificates, used in addition to the operating system's store, for example a root CA from an MCP instance |

See [External requirements](EXTERNAL-REQUIREMENTS.md#mcp-certificates) for how to get a certificate. Keep certificates and keys outside Git.

### `[ship]`

| Key | Required for | Example | Meaning |
| --- | --- | --- | --- |
| `vessel_id` | S-212 | `"urn:mrn:iala:ship:example:vessel"` | Your vessel's MRN, used as the sender of S-212 messages. The VTS service must recognise it |

Keep this key present. The VTS module reads it when the client starts, and the client may stop if it is missing.

### `[locations]`

The VTS locations offered when you create a traffic-clearance request. Each key is a group name shown in the dialog; its value is a list of location URNs:

```toml
[locations]
ExamplePort = [ "urn:mrn:iala:location:xxyyy:berth1",
                "urn:mrn:iala:location:xxyyy:berth2" ]
```

Use the URNs that your VTS service accepts. The values in the tracked `config.toml` are from the MaDaMe pilot area and work only with that service.

## Settings with no effect in this release

These sections belonged to the chart-enabled NoviaECS system. The client still reads some of them, but the code that used them was removed with the chart SDK. They can stay as they are.

| Section | Keys | Former use |
| --- | --- | --- |
| `[nautilus]` | `app_id`, `customer_id`, `nautilus_path` | Chart SDK licence and runtime location |
| `[charts]` | `allApproaches`, `turkuHarbor`, `generalCharts`, `danishCharts`, `nw_alpha` | Chart folders and warning-overlay transparency |
| `[nmea]` | `comport`, `baud`, `VTG`, `HDT`, `GGA` | Own-ship position and heading from a serial NMEA device |

## Settings that are not in `config.toml`

| Setting | Where it is set |
| --- | --- |
| Service registries searched by the MSR menu | `src/MSR/MSRMainModule.cpp`. Two are set: a national test registry and the MCP Public Demonstrator (`https://msr.maritimeconnectivity.net/api/secom`) |
| Data and database folders | Fixed paths relative to `build`: `../data/NW`, `../data/AtoN`, `../data/VTS`, `../data/DB` |
| S-212 schema | Fixed path `../schemas/S212.xsd`; see [External requirements](EXTERNAL-REQUIREMENTS.md#s-212-schema) |
| VTS polling interval | 2.5 s, in `src/mainUI/vtsdialog.cpp` |
| Network log folder | `netlogs/` in the working directory (`build/netlogs/`) |

## Local files the client creates

All paths are relative to the repository root. Delete `data/DB` to start again with empty databases; the files in `data/NW` and `data/AtoN` are parsed again at the next start.

| Path | Contents |
| --- | --- |
| `data/NW/` | S-124 files, downloaded or copied in. A live update moves the previous files to `data/NW/backup/` |
| `data/AtoN/` | S-125 files, with `backup/` as for S-124 |
| `data/VTS/` | S-212 messages sent and received |
| `data/DB/NW.db`, `aton.db`, `msr.db`, `vts_messages.db` | SQLite databases: which files were parsed, discovered services, and VTS threads |

The backend's storage is separate; see [SECOM backend](SECOM-BACKEND.md#4-run).
