#!/usr/bin/env python3

import argparse
import hashlib
import json
from datetime import datetime, timezone
from pathlib import Path
from urllib.parse import quote

REPO = "OpenVibe/ovbs-studio"
PRODUCT = "OpenVibe Studio"
SCHEMA_VERSION = 1

ASSETS = {
    "windows_x64_installer": "OpenVibe-Windows-x64-Installer.exe",
    "windows_x64_portable": "OpenVibe-Windows-x64-Portable.zip",
    "macos_arm64": "OpenVibe-macOS-Apple.dmg",
    "macos_x64": "OpenVibe-macOS-Intel.dmg",
    "linux_deb_x64": "OpenVibe-Linux-x86_64.deb",
    "source": "OpenVibe-Source.tar.gz",
}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--release-dir", required=True)
    parser.add_argument("--version", required=True)
    parser.add_argument("--tag", required=True)
    parser.add_argument("--commit", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    release_dir = Path(args.release_dir).resolve()
    output = Path(args.output).resolve()
    tag_encoded = quote(args.tag, safe="")

    missing = [filename for filename in ASSETS.values() if not (release_dir / filename).is_file()]
    if missing:
        raise SystemExit("Missing release assets required by update manifest: " + ", ".join(missing))

    assets = {}
    for key, filename in ASSETS.items():
        path = release_dir / filename
        assets[key] = {
            "filename": filename,
            "url": f"https://github.com/{REPO}/releases/download/{tag_encoded}/{filename}",
            "bytes": path.stat().st_size,
            "sha256": sha256(path),
        }

    manifest = {
        "schema_version": SCHEMA_VERSION,
        "product": PRODUCT,
        "channel": "prerelease" if "-" in args.version else "stable",
        "version": args.version,
        "tag": args.tag,
        "commit": args.commit,
        "published_at": datetime.now(timezone.utc).isoformat(timespec="seconds").replace("+00:00", "Z"),
        "release_url": f"https://github.com/{REPO}/releases/tag/{tag_encoded}",
        "assets": assets,
    }

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
