"""Prepare verifiable corresponding sources from the vcpkg records used to build.

This reads cached source archives/recipes; it never downloads or installs software.
The output and its manifest are inputs to the Windows CPack distribution.
"""

import argparse
import hashlib
import json
from pathlib import Path
import zipfile


def digest(path, algorithm="sha256"):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, algorithm).hexdigest()


def prepare(args):
    records = sorted((args.installed / "share").glob("*/vcpkg.spdx.json"))
    if not records:
        raise ValueError("No installed vcpkg SPDX records found")
    ports, resources, notices = {}, {}, {}
    for record in records:
        data = json.loads(record.read_text(encoding="utf-8"))
        port = next(p for p in data["packages"] if p["SPDXID"] == "SPDXRef-port")
        name = port["name"]
        tree = port["downloadLocation"].rsplit("@", 1)[-1]
        recipe = args.port_cache / tree
        if not recipe.is_dir():
            raise ValueError(f"Missing cached port recipe: {name} ({tree})")
        recipe_files = []
        for item in data["files"]:
            if not item["SPDXID"].startswith("SPDXRef-port-file-"):
                continue
            relative = Path(item["fileName"])
            if relative.is_absolute() or ".." in relative.parts:
                raise ValueError(f"Unsafe recipe path for {name}")
            path = recipe / relative
            expected = next(c["checksumValue"] for c in item["checksums"] if c["algorithm"] == "SHA256")
            if digest(path) != expected:
                raise ValueError(f"Port recipe changed: {name}/{relative}")
            recipe_files.append((path, f"ports/{name}/{relative.as_posix()}"))
        copyright_file = record.parent / "copyright"
        if not copyright_file.is_file():
            raise ValueError(f"Missing copyright notice for {name}")
        notices[name] = (copyright_file, record)
        ports[name] = {
            "version": port["versionInfo"],
            "recipe_tree": tree,
            "spdx_sha256": digest(record),
            "recipe_files": recipe_files,
            "sources": [],
        }
        for resource in data["packages"]:
            if not resource["SPDXID"].startswith("SPDXRef-resource-"):
                continue
            checksum = next(c["checksumValue"] for c in resource["checksums"] if c["algorithm"] == "SHA512")
            resources.setdefault(checksum, {"resource": resource, "ports": []})["ports"].append(name)

    # Match content, rather than trusting a cache filename to identify a version.
    missing = set(resources)
    for archive in sorted(args.downloads.iterdir()):
        if not archive.is_file() or archive.suffix.lower() not in {".gz", ".xz", ".bz2", ".zip", ".whl", ".zst"}:
            continue
        checksum = digest(archive, "sha512")
        if checksum in missing:
            resources[checksum]["archive"] = archive
            missing.remove(checksum)
        if not missing:
            break
    if missing:
        names = [resources[key]["resource"]["name"] for key in sorted(missing)]
        raise ValueError("Required cached source archives are missing: " + ", ".join(names))

    args.output.parent.mkdir(parents=True, exist_ok=True)
    manifest = {"format": 1, "ports": {}, "source_archives": [], "files": {}}
    with zipfile.ZipFile(args.output, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=1) as output:
        def add(path, destination):
            output.write(path, destination)
            manifest["files"][destination] = digest(path)

        for checksum, item in sorted(resources.items()):
            archive = item["archive"]
            destination = "upstream/" + archive.name
            add(archive, destination)
            source = {"file": destination, "sha512": checksum,
                      "upstream": item["resource"]["downloadLocation"]}
            manifest["source_archives"].append(source)
            for name in item["ports"]:
                ports[name]["sources"].append(source)
        for name, port in sorted(ports.items()):
            for path, destination in port.pop("recipe_files"):
                add(path, destination)
            copyright_file, record = notices[name]
            add(copyright_file, f"notices/{name}/copyright")
            add(record, f"notices/{name}/vcpkg.spdx.json")
            manifest["ports"][name] = port
        # vcpkg's own scripts/recipes retain its MIT licence.
        add(args.vcpkg_root / "LICENSE.txt", "vcpkg/LICENSE.txt")
        add(args.vcpkg_root / "NOTICE.txt", "vcpkg/NOTICE.txt")
        for path in sorted((args.vcpkg_root / "scripts").rglob("*")):
            if path.is_file() and not path.is_symlink():
                add(path, "vcpkg/" + path.relative_to(args.vcpkg_root).as_posix())
        for path in sorted((args.vcpkg_root / "triplets").rglob("*.cmake")):
            add(path, "vcpkg/" + path.relative_to(args.vcpkg_root).as_posix())
        add(args.triplet_file, "triplets/" + args.triplet_file.name)
        project = Path(__file__).resolve().parents[1]
        add(project / "vcpkg.json", "application/vcpkg.json")
        instructions = """# Rebuilding and replacing the distributed libraries

The upstream/ files are the original source archives used by the installed
vcpkg records. ports/ contains the exact checked recipes and patches, and
notices/ retains each library's terms and SPDX record. manifest.json records
versions, SHA-512 source hashes and SHA-256 recipe hashes.

Use Visual Studio with C++ tools, Git and the CMake versions described in the
application's Getting Started guide. Obtain the vcpkg tool from its official
repository; use the version recorded in notices/*/vcpkg.spdx.json creationInfo.
The matching scripts and standard triplets are included under vcpkg/.

In a short working directory on Windows, point VCPKG_DOWNLOADS at the extracted
upstream/ folder. Run vcpkg install --triplet x64-windows with
--x-manifest-root=application, --overlay-ports=ports and --overlay-triplets=triplets.
The supplied overlay triplet records the original linkage/compiler selection;
adjust the compiler selection if rebuilding with another compatible toolset.
The recipes apply the recorded modifications to the upstream archives.

The application uses shared Qt libraries. Replace DLLs beside the executable
and its plugins with matching x64/Qt-ABI replacements; no signature or launcher
prevents using modified libraries. Follow library build instructions in the
source archives for other build systems. Keep all applicable copyright and
licence notices when redistributing modified versions.
"""
        output.writestr("BUILDING.md", instructions)
        output.writestr("manifest.json", json.dumps(manifest, indent=2))
    manifest["archive_sha256"] = digest(args.output)
    manifest_path = Path(str(args.output) + ".manifest.json")
    manifest_path.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    print(f"Prepared {args.output.name}: {len(ports)} ports, {len(resources)} verified source archives")
    print(f"SHA-256: {manifest['archive_sha256']}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ["installed", "downloads", "port-cache", "vcpkg-root", "triplet-file", "output"]:
        parser.add_argument("--" + name, required=True, type=Path)
    try:
        prepare(parser.parse_args())
    except (ValueError, OSError, StopIteration, KeyError) as error:
        parser.exit(1, f"Cannot prepare corresponding sources: {error}\n")
