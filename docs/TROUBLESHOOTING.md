# Troubleshooting

This page collects errors met while setting up `s100-sampler` on Windows, with their cause and fix. Each entry says how likely you are to meet it, and whether we saw it ourselves or expect it from reading the code.

| Likelihood | Meaning |
| --- | --- |
| **Likely** | Expected on most first set-ups, or for everyone in a common situation |
| **Possible** | Depends on how your PC is set up |
| **Rare** | Needs an unusual set-up or a later change to the code |

**Seen** means it happened during our own set-ups in 2026. **Expected** means it follows from the code or configuration, but we have not seen it.

When you report a new problem, include the repository revision (`git rev-parse HEAD`), the exact command, and the first error message. Later messages are usually consequences of the first.

## Configuring and building the client

### `Could not create named generator Visual Studio 18 2026`

**Likely** if you have a standalone CMake as well as Visual Studio. **Seen.**

**Cause:** the `cmake` found first on `PATH` is older than your Visual Studio and does not know its generator. Typing `cmake` in a plain Command Prompt or PowerShell finds the standalone copy (for example `C:\Program Files\CMake`).

**Fix:** use the CMake that ships with Visual Studio. Open **Developer PowerShell for VS**, or call it by its full path:

```bat
set CMAKE="C:\Program Files\Microsoft Visual Studio\18\<Edition>\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
%CMAKE% --version
```

`where.exe cmake` lists every copy on `PATH`, the one used first.

### `No CMAKE_CXX_COMPILER could be found`

**Possible**: after vcpkg has finished, when the configure command sets a compiler version. **Seen.**

**Cause:** `build\CMakeFiles\CMakeConfigureLog.yaml` shows `error MSB4019: The imported project "…\Insiders,version=…\VC\Auxiliary\Build\14.52\Microsoft.VCToolsVersion.14.52.props" was not found`. CMake built that path from `-T version=…` and a `CMAKE_GENERATOR_INSTANCE` value containing `,version=`.

**Fix:** remove `-T version=…` if Visual Studio's default toolset is complete, delete `build\CMakeCache.txt` and `build\CMakeFiles`, and configure again. The installed packages in `build\vcpkg_installed` are kept.

### `build is not a directory`

**Likely** after any failed configure. **Seen.**

**Cause:** the configure step failed, so no build folder was created. This is a consequence, not the problem.

**Fix:** scroll up to the first `CMake Error` of the configure step and fix that. Delete `build` before configuring again.

### `Generator Visual Studio 18 2026 could not find any instance of Visual Studio`

**Possible**: only with a Preview or Insiders edition of Visual Studio. **Seen.**

**Cause:** CMake does not pick a pre-release Visual Studio unless told where it is.

**Fix:** add the installation folder and its version to the configure command:

```text
-DCMAKE_GENERATOR_INSTANCE="C:/Program Files/Microsoft Visual Studio/18/Insiders,version=18.12.12316.0"
```

Without `version=`, CMake may still report that the instance "is not known to the Visual Studio Installer". The version is the `InstallationVersion` line in `Common7\IDE\devenv.isolation.ini` inside the installation folder. Delete `build` before configuring again, because CMake remembers the failed setting.

### vcpkg fails on its first package with `LNK1104: cannot open file 'MSVCRTD.lib'`

**Possible**: after a Visual Studio update that installed a new compiler only partly. **Seen.**

**Cause:** several MSVC versions are installed, and vcpkg uses the newest one. If that version's `VC\Tools\MSVC\<version>\lib\x64` folder has no runtime libraries, every package fails to link. The message is in `build\vcpkg_installed\vcpkg\blds\<package>\config-x64-windows-out.log`.

**Check:** list `C:\Program Files\Microsoft Visual Studio\18\<Edition>\VC\Tools\MSVC\`. A version folder without `crt` and `atlmfc`, or whose `lib\x64` lacks `msvcrt.lib`, is incomplete.

**Fix, preferred:** in the Visual Studio Installer, choose **Modify**, then **Individual components**, and either install the "MSVC … x64/x86 build tools" for that version completely or remove it.

**Fix, without changing Visual Studio:** tell vcpkg and the project to use a complete version. Create a folder outside the repository, for example `C:\Users\<you>\vcpkg-triplets`, with a file `x64-windows.cmake`:

```cmake
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_PLATFORM_TOOLSET_VERSION 14.52)
```

Then add to the configure command:

```text
-DVCPKG_OVERLAY_TRIPLETS=C:/Users/<you>/vcpkg-triplets
```

Replace `14.52` with your complete version. The project itself uses Visual Studio's default toolset, named in `VC\Auxiliary\Build\Microsoft.VCToolsVersion.default.txt`. Only if that default is the incomplete version, also add `-T version=14.52`. Do not combine `-T version=` with a `CMAKE_GENERATOR_INSTANCE` that contains `,version=`: CMake then looks for the toolset in a wrong path and reports `No CMAKE_CXX_COMPILER could be found`.

### The Qt build fails with `C1083: Cannot open include file: '../../../../../src/…'`

**Likely**, unless the checkout is in a short folder such as `C:\src\s100-sampler`. **Seen** on two PCs.

**Symptom:** vcpkg stops in `qt5-base` after several minutes. `build\vcpkg_installed\vcpkg\blds\qt5-base\build-x64-windows-dbg-out.log` contains, for example:

```text
fatal error C1083: Cannot open include file: '../../../../../src/platformsupport/fontdatabases/freetype/qfreetypefontdatabase_p.h': No such file or directory
```

**Cause:** Qt's headers include each other through long relative paths. Inside `build\vcpkg_installed\vcpkg\blds\qt5-base\…` these exceed Windows' 260-character path limit, which the MSVC compiler does not lift. The file exists; the compiler cannot open it by that path.

**Fix:** let vcpkg build in a short folder by adding this to the configure command:

```text
-DVCPKG_INSTALL_OPTIONS=--x-buildtrees-root=C:/vcpkg-bt
```

Packages that were already built are restored from the cache. Alternatively, map the checkout to a drive letter with `subst S: "%CD%"` and build from `S:\`; the mapping is lost when Windows restarts.

### A vcpkg package fails because a file is locked

**Possible**: with antivirus scanning, cloud-synced folders (OneDrive), or an editor watching the folder. **Seen** twice on two PCs, both times while building `pkgconf`.

**Symptom:** the package's `config-…-err.log` ends with `PermissionError: [WinError 32] The process cannot access the file because it is being used by another process`, often for `build.ninja`.

**Cause:** another program (usually real-time virus scanning) holds a file that the build tool is replacing.

**Fix:** run the configure command again. Packages that were already built are restored from vcpkg's cache, so a retry is quick. If it keeps failing, close editors that have the folder open, build from a folder outside synced locations, or ask your IT support to exclude the `build` folder from real-time scanning.

### The first configure takes a very long time

**Likely.** **Seen.**

This is normal: vcpkg builds about 37 packages from source, Qt 5 among them, which can take an hour or more. Later builds reuse vcpkg's binary cache in `%LOCALAPPDATA%\vcpkg\archives` and are much faster.

### `xmllint not found`

**Rare** with this repository's `vcpkg.json`. **Seen** before the manifest enabled the tool.

**Cause:** libxml2 was installed without its command-line tools.

**Fix:** use the vcpkg toolchain; `vcpkg.json` asks for `libxml2[tools]`. With a system libxml2, install its `xmllint` and put it on `PATH`.

## Running the client

### "Windows protected your PC" when starting the client from the Windows package

**Likely** on a first start. **Expected.**

**Cause:** the program is not code-signed, and Windows SmartScreen warns about unsigned programs it has seen on few PCs.

**Fix:** choose **More info → Run anyway**, if you downloaded the package from this repository's **Releases** page.

### `Access is denied` when starting the client from the Windows package

**Possible** on a PC managed by an organisation. **Seen.**

**Symptom:** PowerShell reports `Program 's100-sampler.exe' failed to run: Access is denied`, although the same program built on that PC runs from `build\Release`.

**Cause:** a Microsoft Defender attack-surface-reduction rule set by the organisation, such as **Use advanced protection against ransomware**, blocks programs that are unsigned and new to Windows. Programs built on the PC itself are not affected. **Windows Security → Virus & threat protection → Protection history**, or event 1121 in the Windows Defender event log, shows the block.

**Fix:** ask your IT administrator for an exception, use a PC without that policy, or build the client from source.

### The window opens and closes immediately

**Rare** with the current source. **Seen** with the original release.

**Cause:** `src/main.cpp` returned before starting Qt's event loop. Fixed in this repository (`return app.exec();`).

### Icons are missing, or `Parsing failed` at start-up

**Likely** if you start the executable by double-clicking it. **Expected.**

**Cause:** the client finds `config.toml`, `resources/` and `data/` relative to its working directory, which must be `build`, not `build\Release`.

**Fix:**

```bat
cd build
Release\s100-sampler.exe
```

### "Some network features disabled" at start-up

**Likely** on a new set-up. **Expected.**

**Cause:** `certpath` or `keypath` is missing in `config.toml`. The client works offline, but cannot download from live services. See [External requirements](EXTERNAL-REQUIREMENTS.md#mcp-certificates).

### The NW or AtoN list is empty

**Likely** the first time. **Expected.**

**Check, in order:**

1. The files are in `data/NW` (S-124) or `data/AtoN` (S-125) in the repository root.
2. Their extension is lower-case `.xml` or `.gml`. Files from the Canadian Coast Guard trial end in `.GML` and must be renamed.
3. The client was restarted, or **Manual NW update** was run, after the files were copied.
4. The console shows no `Error processing file` lines. A file in another product edition may not parse.

S-125: only AtoN with a status change are listed, so a valid dataset can still give an empty list.

### "Load Services" finds nothing, or the app closes

**Likely** for the national test registry. **Seen.**

**Symptom:** after **Load Services**, the terminal shows `Error fetching MSR list: cURL request failed: Could not resolve hostname`. Builds before October 2026 then closed without a message.

**Cause:** the registry's address cannot be reached from your network. The national test registry set in `src/MSR/MSRMainModule.cpp` was not reachable from a public network in October 2026; it may need the provider's network or may have moved. The MCP Public Demonstrator was reachable.

**Fix:** use the MCP Public Demonstrator, or edit the registry address in `src/MSR/MSRMainModule.cpp`. The current source shows an empty result instead of closing.

### HTTPS requests fail with a certificate error

**Possible**: with test servers. **Expected.**

**Cause:** the client verifies server certificates (`verify_tls = true`). A test server with a self-signed certificate, or one issued by an MCP certificate authority that Windows does not trust, fails this check. The details are in `build\netlogs\`.

**Fix:** add the issuing CA as a PEM file with `ca_bundle` in `config.toml`. For a test server only, you can set `verify_tls = false`. See [Configuration](CONFIGURATION.md).

### Downloads fail although the server certificate is fine

**Possible.** **Expected.**

**Cause:** the service does not accept your client certificate, or `certpath`/`keypath` point to the wrong files. Paths are relative to `build`.

**Fix:** check the paths, and ask the service provider whether your MCP identity has access.

### "Message not sent" with a schema-validation error

**Likely** on a new set-up that uses S-212. **Expected.**

**Cause:** `schemas/S212.xsd` is not in the repository, and a message that fails validation is not sent. See [External requirements](EXTERNAL-REQUIREMENTS.md#s-212-schema). The Windows package also lacks `xmllint`, which the check runs from the build PC's vcpkg folder, so on other PCs it cannot send S-212 messages; build from source.

### "Could not connect to server" or "Empty response received"

**Likely** when the backend is not running. **Seen.**

**Symptom:** the terminal repeats `Error at VTS message pulling: cURL request failed: Could not connect to server` and `Error fetching file list: …` every few seconds.

**Cause:** the client polls the backend at `api_endpoint` every 2.5 s while an S-212 service is active, and checks it once at start-up. Builds before October 2026 polled from start-up even without an S-212 service. A single message at start-up is harmless if you do not use S-212.

**Fix:** start the backend ([SECOM backend](SECOM-BACKEND.md#4-run)) and check that `api_endpoint` matches its address and port. "Empty response received" from an older backend build is fixed in this repository.

## Building and running the backend

### `mvn` is not recognized as the name of a cmdlet

**Possible**: Maven has no Windows installer and is not on `PATH` unless you add it. **Seen.**

**Cause:** Maven is not installed, or its `bin` folder is not on `PATH`. A copy may already exist, for example one set up by an editor extension under `%USERPROFILE%\.maven`.

**Fix:** add the `bin` folder to `PATH` in the current window, then run `mvn -version`:

```powershell
$env:Path = "C:\path\to\apache-maven\bin;$env:Path"
```

To make it permanent, add the folder to `Path` under **Edit the system environment variables → Environment Variables**.

### Maven cannot resolve `secom-core-jakarta` or `secom-springboot3` 0.0.24

**Likely** on a first set-up. **Expected.**

**Cause:** `secom/pom.xml` uses the patched SECOMLib from your local Maven repository and names no remote repository for it, so it fails until you have built it.

**Fix:** build SECOMLib 0.0.24 from source with the patch in this repository: [SECOM backend, step 1](SECOM-BACKEND.md#1-build-secomlib-0024).

### `cannot find symbol: variable S201` in `SecomUpload.java`

**Possible** if your local Maven repository holds the published SECOMLib 0.0.24. **Seen.**

**Cause:** the published 0.0.24 lacks the `S201` data-product type; the patch adds it. Revisions of this repository before 28 October 2026 listed the publisher's repository in `secom/pom.xml`, so Maven downloaded the unpatched library whenever that repository was reachable.

**Fix:** do [SECOM backend, step 1](SECOM-BACKEND.md#1-build-secomlib-0024). Its `install` replaces the downloaded copy. Then build the backend again.

### `git apply` reports `can't open patch`

**Possible.** **Seen.**

**Cause:** `git -C C:\tools\SECOMLib` runs Git inside the SECOMLib folder, so a relative path such as `docs\backend\secomlib-0.0.24-fixes.patch` is looked up there.

**Fix:** pass the full path, as in step 1: `$patch = (Resolve-Path docs\backend\secomlib-0.0.24-fixes.patch).Path`, run from the root of this repository.

### Four SECOMLib tests fail in `SecomPemUtilsTest`

**Likely** on Windows if Git converts line endings (`core.autocrlf=true`). **Seen.**

**Cause:** the test fixtures are checked out with Windows line endings, and the comparison fails.

**Fix:** apply `docs/backend/secomlib-0.0.24-fixes.patch`, which makes the tests ignore line endings.

### Compilation errors in `SecomAck.java` or `SecomController.java`

**Rare** with the current source. **Seen** with the original release.

**Cause:** the original backend used method names from a different SECOMLib version (`setMessage`, `SHA2_256_WITH_ECDSA`). Fixed in this repository.

### `mvn -version` reports a Java version other than 17

**Possible**: when several JDKs are installed. **Seen.**

**Cause:** Maven uses the JDK in `JAVA_HOME`, or the first `java` on `PATH` when `JAVA_HOME` is empty. If JDK 17 is not installed, the `Get-ChildItem "C:\Program Files\Microsoft\jdk-17*"` line in the backend guide finds nothing and leaves `JAVA_HOME` empty without an error, so Maven quietly uses another Java.

**Fix:** install JDK 17 (`winget install --id Microsoft.OpenJDK.17 -e`), set `JAVA_HOME` again in the same terminal, and check that `$env:JAVA_HOME` prints a folder before running Maven. A newer JDK has not been tested.

### Port 8443 is already in use

**Rare.** **Expected.**

**Fix:** stop the other process, or change `server.port` in the backend profile and `api_endpoint` in the client's `config.toml` to match.

### Warning: `File encoding has not been set, using platform encoding windows-1252`

**Likely.** **Seen.** A second warning, `Using platform encoding (Cp1252 actually) to copy filtered resources`, has the same cause.

**Cause:** `secom/pom.xml` does not set a source encoding. The backend sources and resources are plain ASCII, so the encoding does not change the result.

**Fix:** none needed.
