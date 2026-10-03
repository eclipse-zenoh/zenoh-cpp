#!/usr/bin/env python3
"""Apply or check the repository's pinned clang-format, independently of PATH."""

import argparse
import os
import re
import subprocess
import sys
import venv
from pathlib import Path


# Keep the full version pinned: even patch releases can change formatting.
CLANG_FORMAT_VERSION = "18.1.3"
ROOT = Path(__file__).resolve().parent.parent


def formatter():
    environment = ROOT / ".cache" / f"clang-format-{CLANG_FORMAT_VERSION}"
    windows = os.name == "nt"
    binaries = environment / ("Scripts" if windows else "bin")
    python = binaries / ("python.exe" if windows else "python")
    executable = binaries / ("clang-format.exe" if windows else "clang-format")

    if not python.is_file():
        venv.EnvBuilder(with_pip=True).create(environment)
    if not executable.is_file():
        subprocess.run(
            [
                str(python), "-m", "pip", "install", "--disable-pip-version-check",
                "--only-binary=:all:", f"clang-format=={CLANG_FORMAT_VERSION}",
            ],
            check=True,
        )

    version = subprocess.check_output([str(executable), "--version"], text=True).strip()
    match = re.search(r"clang-format version (\d+\.\d+\.\d+)\b", version)
    if match is None or match.group(1) != CLANG_FORMAT_VERSION:
        raise RuntimeError(
            f"Expected clang-format {CLANG_FORMAT_VERSION}, got {version!r}. "
            f"Remove {environment} and retry."
        )
    print(version, flush=True)
    return executable


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="Report violations without editing files")
    args = parser.parse_args()
    executable = formatter()
    files = sorted(
        str(path.relative_to(ROOT))
        for directory in ("include", "tests", "examples")
        for path in (ROOT / directory).rglob("*")
        if path.is_file() and path.suffix.lower() in (".hxx", ".cxx")
    )
    flags = ["--dry-run", "--Werror"] if args.check else ["-i"]
    # Bound command length for Windows, and pass paths as separate arguments.
    result = 0
    for start in range(0, len(files), 100):
        batch = files[start : start + 100]
        status = subprocess.run([str(executable), *flags, *batch], cwd=ROOT).returncode
        if status:
            result = status
    return result


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, RuntimeError, subprocess.CalledProcessError) as error:
        print(f"Formatting failed: {error}", file=sys.stderr)
        sys.exit(1)
