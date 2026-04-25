# astro

A small C++17 geodesy project centered around geographic points on the WGS-84 ellipsoid.

The current executable demonstrates:

- Geographic coordinate storage (latitude, longitude, altitude)
- Conversion to ECEF (Earth-Centered, Earth-Fixed) Cartesian coordinates
- Conversion from ECEF back to geodetic coordinates
- Great-circle distance with the Haversine formula

## Quick Start

Build debug (default):

```bash
./build.sh
```

Build release:

```bash
./build.sh release
```

Run:

```bash
./bin/astro
```

## Documentation

Detailed docs are in the docs folder:

- docs/BUILD.md: build and run workflows
- docs/API.md: public API surface and usage examples
- docs/PROJECT_STRUCTURE.md: repository layout and responsibilities
- docs/GIT_WORKFLOW.md: git helper commands and common workflows

## Requirements

- CMake >= 3.16
- A C++17 compiler (current presets use g++)

## License

See LICENSE.
