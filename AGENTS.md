# Repository Guide

## Project overview

This repository contains two related applications:

- The root project is `s100-sampler`, a C++17/Qt 5 Widgets desktop application for downloading and inspecting S-100-family maritime data.
- `secom/` is a Java 17/Spring Boot backend that provides the custom endpoint behavior expected by the desktop application's VTS features. It is not interchangeable with a generic SECOM endpoint.

The licensed Nautilus chart SDK was removed. Some names, comments, configuration keys, UI actions, and disabled code still reflect the former chart integration. Do not reintroduce SDK assumptions unless the task explicitly requires it.

## Repository map

- `src/main.cpp`: desktop application entry point and global Qt setup.
- `include/mainUI/`, `src/mainUI/`: Qt Widgets windows, dialogs, and generated-UI consumers.
- `include/S100/`, `src/S100/`: shared S-100 data-layer and display abstractions.
- `include/S124/`, `src/S124/`: navigational warning models and persistence.
- `include/S125/`, `src/S125/`: aids-to-navigation models and persistence.
- `include/S212/`, `src/S212/`: VTS messages, repository, and widgets.
- `include/parser/`, `src/parser/`: XML/JSON parsing.
- `include/MSR/`, `src/MSR/`: Maritime Service Registry integration.
- `resources/`: desktop icons and Windows application resources.
- `config.toml`: desktop runtime configuration template/defaults.
- `secom/src/main/java/com/example/secom/`: Spring controllers, services, and JPA records.
- `secom/src/main/resources/application.properties.example`: backend configuration template.
- `docs/`: user and developer documentation (getting started, external requirements, backend, configuration, architecture, limitations, troubleshooting). `docs/backend/` holds the SECOMLib 0.0.24 patch and a local backend profile.

## Build and run

Follow `docs/GETTING-STARTED.md` for the full procedure and `docs/TROUBLESHOOTING.md` for known build errors. In short, the desktop build uses CMake 3.21+, C++17, Qt 5, and the dependencies declared in `vcpkg.json`, whose pinned baseline fixes the package versions. Only Windows has been built and tested. Configure with the vcpkg toolchain and the generator of the installed Visual Studio, using the CMake that ships with it:

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release --parallel 4
```

The first configure builds about 37 packages, Qt among them, and can take an hour. Qt fails on Windows' path-length limit in deep folders; add `-DVCPKG_INSTALL_OPTIONS=--x-buildtrees-root=C:/vcpkg-bt` if needed.

A Linux build (for example with Ninja) has not been tested.

The application currently resolves `../config.toml`, `../resources`, `../data`, and `../schemas` relative to its working directory. A build-directory launch is therefore the expected development layout. Network features require user-provided endpoint and certificate values; never commit real private keys or credentials.

The backend uses Maven and Java 17; follow `docs/SECOM-BACKEND.md`. SECOMLib 0.0.24 must be built from source with `docs/backend/secomlib-0.0.24-fixes.patch` and installed locally; the published 0.0.24 lacks the `S201` data-product type, and `secom/pom.xml` deliberately lists no SECOMLib repository. Then:

```powershell
mvn -f secom/pom.xml package
```

Put local settings in the ignored `secom/config/application-local.properties` (start from `docs/backend/application-local.properties.example`) and run the JAR from `secom/` with `--spring.profiles.active=local`. Backend upload directories are relative to the process working directory.

## Change conventions

- Keep declarations in the matching `include/` subtree and implementations in `src/`.
- Preserve Qt ownership rules and signal/slot wiring. Classes with signals or slots need `Q_OBJECT`; let CMake `AUTOMOC`, `AUTOUIC`, and `AUTORCC` handle generated files.
- Edit `.ui` source files, not generated `ui_*.h` files.
- Prefer RAII and `std::unique_ptr` for new ownership. Existing raw pointers often cross parser/model/Qt ownership boundaries, so establish ownership before changing them.
- Keep S-124, S-125, and S-212 product-specific behavior in their respective modules; shared XML geometry parsing belongs in the S-100 parser layer.
- Treat downloaded XML/GML files and SQLite databases under `data/` as runtime state, not source fixtures, unless a task explicitly adds sanitized test data.
- Preserve existing user configuration when editing `config.toml`; use placeholders for secrets and machine-specific paths.
- Make focused changes. Do not revive large blocks of Nautilus-related commented code as incidental cleanup. The commented SDK calls in `MainWindow` are kept on purpose as a map of the former chart integration; see `docs/ARCHITECTURE.md`.
- Keep each file's existing line endings. Many files are stored with CRLF; converting a whole file creates a diff of every line.
- When a change affects behaviour, configuration or a known issue, update the matching page in `docs/` (in particular `LIMITATIONS.md` and `TROUBLESHOOTING.md`).
- Never commit datasets, certificates, keys, or chart material; see `docs/EXTERNAL-REQUIREMENTS.md` for what stays outside the repository.

## Validation

There is currently no automated C++ test target, no committed Java test suite, and no continuous integration. Validate in proportion to the change:

- For C++ changes, configure and build the affected CMake target. Run the GUI from the build directory when runtime behavior matters.
- For Qt UI changes, verify that AUTOUIC/AUTOMOC completes and manually exercise the affected dialog or action.
- For parser or persistence changes, use sanitized representative XML/GML and a disposable database; do not overwrite user runtime data.
- For backend changes, run `mvn -f secom/pom.xml test` (which at minimum compiles the project) and exercise affected HTTP endpoints when configuration is available.
- If dependencies, certificates, schemas, or local service configuration prevent validation, report that explicitly rather than claiming the build or runtime path passed.

Before handing off, inspect `git diff --check` and `git status --short`, and summarize both validation performed and validation that remains unavailable.
