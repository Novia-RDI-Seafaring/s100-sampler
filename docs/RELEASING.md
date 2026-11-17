# Preparing a Windows distribution

Build the Release target and run [the regression tests](TESTING.md) first. Keep
the manifest's pinned dependencies. The package includes the `xmllint` tool and
its DLLs, third-party notices, and corresponding library sources. The S-212
schema, credentials and runtime datasets remain user-provided.

## Corresponding sources and notices

Use Python 3.11 or later to prepare source materials from the same vcpkg install
used by CMake. The script reads and verifies source-cache and recipe hashes from
the installed SPDX records; it does not download or modify dependencies. Give
the actual cache paths used by your vcpkg installation, for example:

```powershell
python tools/package_sources.py `
  --installed build/vcpkg_installed/x64-windows `
  --downloads "$env:LOCALAPPDATA/vcpkg/downloads" `
  --port-cache "$env:LOCALAPPDATA/vcpkg/registries/git-trees" `
  --vcpkg-root "$env:VCPKG_ROOT" `
  --triplet-file "$env:VCPKG_ROOT/triplets/x64-windows.cmake" `
  --output build/corresponding-sources.zip
```

If an overlay triplet was used, pass that file instead of the standard triplet.
Missing archives or changed recipes stop preparation. Preserve the cache while
preparing a release. The output ZIP and its `.manifest.json` must stay together.

Configure the package to use those verified sources, then build and package:

```powershell
cmake -S . -B build -DS100_SOURCE_BUNDLE="$((Resolve-Path build/corresponding-sources.zip).Path)"
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
Set-Location build
cpack -C Release
```

Use the CMake/CTest/CPack from the same Visual Studio installation. CPack checks
the source archive hash and the installed dependency records. It installs the
source archive inside `sources/`, all installed vcpkg copyright/SPDX records under
`licenses/`, the LGPL/GPL/Apache texts, and `THIRD-PARTY-NOTICES.md`. Retain these
files when distributing the ZIP. See the included source `BUILDING.md` for
rebuilding and replacing the shared libraries.

## Review the actual archive

Unpack into a fresh disposable directory with spaces in its path. Verify the
client, Qt plugins, `bin/xmllint.exe` and its dependencies, all notices, and the
source archive. Run `xmllint` there with an empty PATH and synthetic local XML/XSD
to check both acceptance and rejection. Check the package for runtime data,
credentials and developer paths before uploading. Check the SHA-256 of the
finished archive and retain it with the reviewed revision.

Changing repository visibility and publishing a release are separate decisions.
An existing draft ZIP still contains the old binaries until deliberately replaced.
Do not move an existing public tag. Prefer a new version/tag for a changed
distribution; if an unpublished draft tag is deliberately retargeted, review its
new commit and replace all associated assets together. Packaging never uploads an
asset, changes a tag, or publishes a release.
