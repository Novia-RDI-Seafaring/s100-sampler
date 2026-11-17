# Focused regression tests

Configure and build the client as described in [Getting started](GETTING-STARTED.md).
`BUILD_TESTING` is on by default and adds CTest targets using the same dependencies
and implementation as the client:

```powershell
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
```

The dataset download tests call `Curler::saveAndDecodeRawJSONToFiles` with synthetic
Base64 XML in disposable temporary directories. They check parent-directory
traversal, Windows paths and device names, normal identifiers, updates, the
missing-identifier fallback, and existing symlink destinations. The symlink case
is reported as skipped when the operating system denies creating a file symlink.
No endpoint, certificate, runtime dataset or application database is used.

The XML-validator test runs the bundled `xmllint` with an empty PATH, filenames
containing spaces, a synthetic valid/invalid message and a missing schema. It
checks that schema failures remain blocking after relocating the validator.

These tests cover download filename safety and local XML validation. They do not establish S-100 schema
conformance or a complete S-212 exchange. The project has no continuous integration
or committed Java test suite.
