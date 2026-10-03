# CI

## C++ formatting

CI and local formatting use **clang-format 18.1.3**, pinned in
[`scripts/format.py`](../scripts/format.py). The script installs the exact version
from a prebuilt Python wheel into an isolated environment under `.cache/`, then
checks the executable's version before running it. It requires Python 3 with
`venv` and `pip`; the first run needs internet access. Later runs reuse the
cached formatter. The formatter on your system's `PATH` is never used.

From the repository root, apply formatting with:

```sh
python3 scripts/format.py
```

Run the same check as CI without modifying files:

```sh
python3 scripts/format.py --check
```

On Windows, use `py -3` instead of `python3`. Both commands process all `.hxx`
and `.cxx` files under `include/`, `tests/`, and `examples/`, using the repository's
`.clang-format`. No C/C++ dependency build, submodule checkout, or Docker daemon
is needed.

To upgrade the formatter, change `CLANG_FORMAT_VERSION` in `scripts/format.py`
and apply formatting with the script in the same change. The cache is keyed by
the full version, so local runs and CI switch together.
