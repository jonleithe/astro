# astro

A small C++17 geodesy project centered around geographic points on the WGS-84 ellipsoid.

## Table of Contents

- [Quick Start](#quick-start)
- [Documentation](#documentation)
- [Requirements](#requirements)
- [License](#license)

The current executable demonstrates:

- Geographic coordinate storage (latitude, longitude, altitude)
- Conversion to ECEF (Earth-Centered, Earth-Fixed) Cartesian coordinates
- Conversion from ECEF back to geodetic coordinates
- Great-circle distance with the Haversine formula
- Sun sky-position calculation (azimuth/altitude) for a UTC time and observer location
- Daily solar events in UTC (solar noon and sunrise/sunset when available)

## Quick Start

Build debug (default):

```bash
./build.sh
```

Build release:

```bash
./build.sh release
```

Generate API documentation (requires Doxygen):

```bash
./build.sh docs
```

Run:

```bash
./bin/astro [--read <file>] --lat <deg> --lon <deg> [--alt <m>] [--date YYYY-MM-DD] [--time HH:MM:SS]
```

Name lookup mode:

```bash
./bin/astro --read locations.ini.example --name ny_aalesund
```

Name edit mode (updates `names.ini` by default):

```bash
./bin/astro --name ny_aalesund --lat 78.12345678 --lon 11.98765432 --alt 8.2
```

If the name exists, the program asks for confirmation, comments out the old line,
and appends the new definition.

If omitted, date/time default to current UTC.

If `--read` is used, values are loaded from file first and explicit CLI flags override file values.

If `--name` is used together with `--read`, the read file is treated as a location library with lines like:

```text
name = lat, lon[, alt]
```

Example:

```bash
./bin/astro --lat 78.24490 --lon 15.49369 --alt 0.0 --date 2026-06-21 --time 12:00:00
```

Minimal example (uses current UTC date/time):

```bash
./bin/astro --lat 78.24490 --lon 15.49369
```

Read from file example:

```bash
./bin/astro --read astro.conf.example
```

## Documentation

Detailed docs are in the docs folder:

- [docs/BUILD.md](docs/BUILD.md): build and run workflows
- [docs/API.md](docs/API.md): public API surface and usage examples
- [docs/PROJECT_STRUCTURE.md](docs/PROJECT_STRUCTURE.md): repository layout and responsibilities
- [docs/GIT_WORKFLOW.md](docs/GIT_WORKFLOW.md): git helper commands and common workflows
- [docs/DOXYGEN.md](docs/DOXYGEN.md): generate browsable API documentation

## Requirements

- CMake >= 3.16
- A C++17 compiler (current presets use g++)

## License

See LICENSE.
