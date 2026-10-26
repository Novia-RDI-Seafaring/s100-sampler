# SECOM backend

The companion backend in `secom/` is the ship-side SECOM endpoint. A VTS service uploads its S-212 replies and acknowledgements to it, and the desktop client polls it for new messages. You need it only for S-212; S-124 and S-125 work without it.

The backend is not a generic SECOM endpoint. Besides the SECOM interfaces, it has routes that only this client uses (see [Routes](#routes)), so the client needs this backend and not another SECOM server.

Commands below use PowerShell on Windows, the tested platform.

## Requirements

| Requirement | Tested version |
| --- | --- |
| Java JDK | 17 (Microsoft Build of OpenJDK 17.0.20) |
| Apache Maven | 3.10.0 |
| Git | Any recent version |
| SECOMLib | 0.0.24 (`secom-core-jakarta`, `secom-springboot3`) |

The backend targets Java 17. A newer JDK has not been tested. Before building, check that Maven uses Java 17:

```powershell
$env:JAVA_HOME = "C:\Program Files\Microsoft\jdk-17.0.20.101-hotspot"   # adjust to your JDK
mvn -version
```

The output must report Java version 17.

## 1. Get SECOMLib 0.0.24

`secom/pom.xml` lists the SECOMLib publisher's Maven repository. If Maven can download version 0.0.24 from it, skip to step 2.

That repository refused connections during our set-up, so we built 0.0.24 from the official source instead. To do the same, clone SECOMLib outside this repository and select the revision that declares version 0.0.24:

```powershell
git clone https://github.com/gla-rad/SECOMLib.git C:\tools\SECOMLib
git -C C:\tools\SECOMLib switch --detach e14ad59bf9385430285be5dadc67355fe64a686f
```

Apply the patch from this repository, checking it first:

```powershell
git -C C:\tools\SECOMLib apply --check --ignore-space-change <path-to-s100-sampler>\docs\backend\secomlib-0.0.24-fixes.patch
git -C C:\tools\SECOMLib apply --ignore-space-change <path-to-s100-sampler>\docs\backend\secomlib-0.0.24-fixes.patch
```

The patch makes two changes:

- It adds the `S201` data-product type, which the backend uses for AtoN uploads. The entry is copied from upstream commit `8e23e5ab56910110d76adc12e7152ddb3c63b71d`.
- It normalises line endings in two PEM test fixtures, so that the library's tests pass on Windows.

The result is therefore 0.0.24 plus one addition, not a byte-identical copy of the published release.

Build the two modules and install them into your local Maven repository. This also runs the library's tests:

```powershell
mvn -f C:\tools\SECOMLib\pom.xml -pl secom-springboot3 -am install
```

## 2. Build the backend

From the repository root:

```powershell
mvn -f secom\pom.xml package
```

The result is `secom\target\secom-novia-endpoint-0.9.9.jar`.

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
& "$env:JAVA_HOME\bin\java.exe" -jar .\target\secom-novia-endpoint-0.9.9.jar --spring.profiles.active=local
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

On a new installation these return `{}` and `[]`, which are valid empty results.

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
| Compilation errors in `SecomAck.java` or `SecomController.java` | Use a revision of this repository that includes the SECOMLib 0.0.24 API fixes |
| `mvn -version` reports a Java version other than 17 | Set `JAVA_HOME` to a JDK 17 in the same terminal |
| Port 8443 already in use | Stop the other process, or change `server.port` and the client's `api_endpoint` |
| Client shows "Empty response received" | Use a revision where `/v1/ack/getAllTypes` returns `{}` for no records |
