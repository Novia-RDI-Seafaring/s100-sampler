# Third-party software and corresponding sources

s100-sampler's original code is MIT-licensed. Third-party software retains its own
copyright and licence terms. The SECOMLib compatibility patch in `docs/backend/`
is Apache-2.0; see `licenses/Apache-2.0.txt` and the patch header.

## Windows distribution

The client dynamically links Qt 5 (Core, Gui, Widgets, Svg and associated plugins)
under LGPLv3. The full LGPLv3 and GPLv3 texts are in `licenses/`. You may replace
these DLLs with compatible modified versions and run the resulting application.
The project imposes no restriction on reverse engineering for debugging changes
to these libraries. Keep the matching DLL architecture and Qt ABI.

The ZIP includes `sources/corresponding-sources.zip`: the exact upstream source
archives identified by the installed vcpkg records, the port recipes and patches,
and build instructions. `sources/manifest.json` records their checksums and the
dependency versions. This distribution uses accompanying corresponding source;
it does not rely on an unsupported promise to supply source later. Redistribute
that source archive with the binary package.

Complete dependency copyright and licence texts are installed under
`licenses/<port>/copyright`, with each port's SPDX provenance record. This covers
the complete installed dependency set, including:

- Qt Base, SVG and Image Formats (LGPLv3), and any additional Qt build dependencies;
- OpenSSL (Apache-2.0), curl (curl/ISC/BSD notices), libxml2 (MIT), and libiconv
  libraries (LGPL; its separate GPL command-line program is not shipped);
- SQLite/SQLiteCpp, pugixml, cppcodec, nlohmann/json and toml++;
- Brotli, bzip2, double-conversion, FreeType, HarfBuzz, libpng, md4c, PCRE2, zlib
  and the remaining dependencies identified by the included records.

Qt's embedded third-party sources retain their own notices inside the accompanying
source archive. Microsoft Visual C++ runtime DLLs are redistributed under the
Visual Studio runtime redistribution terms; they are not relicensed by this
project's MIT licence.

## Building and redistributing

See [docs/RELEASING.md](docs/RELEASING.md) for preparation and verification. CPack
refuses to create the Windows distribution without matching source materials and
dependency notices. A non-vcpkg/system-library distributor must prepare equivalent
licence and corresponding-source materials for the libraries actually shipped.

References: [Qt LGPL obligations](https://www.qt.io/development/open-source-lgpl-obligations),
[Apache-2.0](https://www.apache.org/licenses/LICENSE-2.0), and each dependency's
included licence text.
