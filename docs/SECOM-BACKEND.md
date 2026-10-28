# SECOM backend

The companion backend in `secom/` is the ship-side SECOM endpoint. A VTS service uploads its S-212 replies and acknowledgements to it, and the desktop client polls it for new messages. You need it only for S-212; S-124 and S-125 work without it.

The backend is not a generic SECOM endpoint. Besides the SECOM interfaces, it has routes that only this client uses (see [Routes](#routes)), so the client needs this backend and not another SECOM server.

Commands below use PowerShell on Windows, the tested platform.

## Requirements

| Requirement | Tested version |
| --- | --- |
| Java JDK | 17 (Microsoft Build of OpenJDK 17) |
| Apache Maven | 3.10.0 and 3.9.13 |
| Git | Any recent version |
| SECOMLib | 0.0.24 (`secom-core-jakarta`, `secom-springboot3`), built from source with this repository's patch |

The backend targets Java 17. A newer JDK has not been tested. If you have no JDK 17, install one:

```powershell
winget install --id Microsoft.OpenJDK.17 -e
```

Run every command on this page from the root of this repository, in one PowerShell window. First point that window at JDK 17 and, if `mvn` is not on your `PATH`, at Maven's `bin` folder. These settings last until you close the window:

```powershell
$env:JAVA_HOME = (Get-ChildItem "C:\Program Files\Microsoft\jdk-17*" | Select-Object -First 1).FullName
$env:JAVA_HOME                                                  # must print a folder; if empty, JDK 17 is not installed there
$env:Path = "$env:JAVA_HOME\bin;$env:Path"
# $env:Path = "C:\path\to\apache-maven\bin;$env:Path"           # only if mvn is not found
mvn -version
```

The output must report Java version 17.

## 1. Build SECOMLib 0.0.24

This step is always needed. The published SECOMLib 0.0.24 lacks the `S201` data-product type that the backend uses, so the backend does not compile against it. `secom/pom.xml` therefore lists no SECOMLib repository and expects the copy you build here in your local Maven repository.

Clone SECOMLib outside this repository and select the revision that declares version 0.0.24:

```powershell
git clone https://github.com/gla-rad/SECOMLib.git C:\tools\SECOMLib
git -C C:\tools\SECOMLib switch --detach e14ad59bf9385430285be5dadc67355fe64a686f
```

Apply the patch from this repository, checking it first. `git -C` runs Git inside the SECOMLib folder, so the patch needs its full path, which the first line works out:

```powershell
$patch = (Resolve-Path docs\backend\secomlib-0.0.24-fixes.patch).Path
git -C C:\tools\SECOMLib apply --check --ignore-space-change $patch
git -C C:\tools\SECOMLib apply --ignore-space-change $patch
```

The check prints nothing when the patch fits.

The patch makes two changes:

- It adds the `S201` data-product type, which the backend uses for AtoN uploads. The entry is copied from upstream commit `8e23e5ab56910110d76adc12e7152ddb3c63b71d`.
- It normalises line endings in two PEM test fixtures, so that the library's tests pass on Windows.

The result is therefore 0.0.24 plus one addition, not a byte-identical copy of the published release.

Build the two modules and install them into your local Maven repository. This also runs the library's tests:

```powershell
mvn -f C:\tools\SECOMLib\pom.xml -pl secom-springboot3 -am install
```

It ends with `BUILD SUCCESS`. If an unpatched 0.0.24 was downloaded into your local Maven repository earlier, this install replaces it.

## 2. Build the backend

```powershell
mvn -f secom\pom.xml package
```

The result is `secom\target\secom-novia-endpoint-1.0.0.jar`. The warnings about platform encoding are harmless: the backend sources and resources are plain ASCII.

## 3. Configure

Spring Boot reads a profile from `secom/config/` when the backend starts in `secom/`. For local testing, copy the example profile from this repository:

```powershell
New-Item -ItemType Directory -Path secom\config -Force
Copy-Item docs\backend\application-local.properties.example secom\config\application-local.properties
```

It contains:

```properties
server.address=127.0.0.1
server.port=8443
server.ssl.enabled=false

spring.datasource.url=jdbc:h2:file:./data/secom
spring.datasource.driver-class-name=org.h2.Driver
spring.datasource.username=sa
spring.datasource.password=
spring.jpa.hibernate.ddl-auto=update
spring.h2.console.enabled=false
```

This profile serves plain HTTP on the loopback address only. It lets you test the connection between the client and the backend on one machine. It cannot receive messages from an external VTS service.

**For a real S-212 exchange,** run the backend on a host the VTS service can reach, with TLS enabled. `secom/src/main/resources/application.properties.example` shows the TLS and mutual-TLS settings (`server.ssl.*`); supply your own keystore and truststore. In the MaDaMe pilots the backend ran on a cloud virtual machine.

Keep `secom/config/`, keystores and the database out of Git.

## 4. Run

```powershell
Set-Location secom
& "$env:JAVA_HOME\bin\java.exe" -jar .\target\secom-novia-endpoint-1.0.0.jar --spring.profiles.active=local
```

Wait for `Started SecomNovia` and Tomcat listening on port 8443. Keep this terminal open while you use the client.

The backend stores its data relative to `secom/`:

| Path | Contents |
| --- | --- |
| `data/secom*.db` | H2 database with file, acknowledgement and AtoN records |
| `uploads/` | Received S-212 messages |
| `aton_uploads/` | Received S-125/S-201 uploads |
| `route_uploads/` | Received S-421 route uploads |

Check that it responds, from a second terminal:

```powershell
Invoke-RestMethod http://127.0.0.1:8443/v1/ack/getAllTypes
Invoke-RestMethod http://127.0.0.1:8443/v1/files/all
```

On a new installation these return `{}` and `[]`, which are valid empty results; PowerShell shows them as a blank line and as nothing.

Steps 1 to 4 were last tested on Windows 11 on 28 October 2026, with Microsoft Build of OpenJDK 17 and Maven 3.9.13.

Then set `api_endpoint = "http://127.0.0.1:8443"` under `[network]` in the client's `config.toml`.

Stop the backend with **Ctrl+C** after closing the client.

## Routes

**SECOM interfaces,** used by the VTS service or other SECOM clients:

| Method and path | Purpose |
| --- | --- |
| `GET /v1/capability` | SECOM Capability |
| `GET /v1/ping` | SECOM Ping |
| `POST /v1/object` | SECOM Upload: receives S-212 messages, S-125/S-201 AtoN and S-421 routes |
| `GET /v1/object` | SECOM Get |
| `GET /v1/object/summary` | SECOM Get Summary |
| `POST /v1/acknowledgement` | SECOM Acknowledgement: stores delivered/opened acknowledgements |
| `POST /v1/subscription/notification` | SECOM Subscription Notification |

**Client routes,** used only by the desktop client:

| Method and path | Purpose |
| --- | --- |
| `GET /v1/files` | List identifiers of received messages |
| `GET /v1/files/{uuid}` | Download one received message |
| `GET /v1/files/undownloaded` | Contents of messages not yet downloaded |
| `GET /v1/files/all` | Contents of all received messages |
| `GET /v1/ack/getType/{transactionIdentifier}` | Acknowledgement state of one message |
| `GET /v1/ack/getAllTypes` | Acknowledgement states of all messages |
| `GET /v1/atons`, `GET /v1/atons/{uuid}` | List and download received AtoN uploads |

## Limitations

- Messages are not signed: the client fills the SECOM signature fields with placeholder values. Signature checking in the backend has not been verified.
- The local profile has no TLS and no authentication.
- There are no automated tests for the backend in this repository.

## Troubleshooting

More detail, including how likely each error is, is in [Troubleshooting](TROUBLESHOOTING.md#building-and-running-the-backend).

| Symptom | Check |
| --- | --- |
| Maven cannot resolve `secom-core-jakarta` or `secom-springboot3` 0.0.24 | Build SECOMLib from source (step 1) |
| `cannot find symbol: variable S201` in `SecomUpload.java` | The unpatched SECOMLib is in your local Maven repository; do step 1 |
| `mvn` is not recognised | Add Maven's `bin` folder to `PATH` in this window, or install Maven |
| `git apply` says `can't open patch` | Use the full patch path (`$patch`) as in step 1 |
| Compilation errors in `SecomAck.java` or `SecomController.java` | Use a revision of this repository that includes the SECOMLib 0.0.24 API fixes |
| `mvn -version` reports a Java version other than 17 | Set `JAVA_HOME` to a JDK 17 in the same terminal |
| Port 8443 already in use | Stop the other process, or change `server.port` and the client's `api_endpoint` |
| Client shows "Empty response received" | Use a revision where `/v1/ack/getAllTypes` returns `{}` for no records |
