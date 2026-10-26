# astro

A small C++17 geodesy project centered around geographic points on the WGS-84 ellipsoid.

## Table of Contents

- [Quick Start](#quick-start)
- [Documentation](#documentation)
- [Requirements](#requirements)
- [License](#license)

The library provides coordinate geometry, while the executable calculates and prints observer and solar information:

- Geographic coordinate storage (latitude, longitude, altitude)
- Conversion to ECEF (Earth-Centered, Earth-Fixed) Cartesian coordinates
- Conversion from ECEF back to geodetic coordinates
- Great-circle distance with the Haversine formula
- Observer-to-target azimuth, signed elevation, and straight-line distance
- Sun sky-position calculation (azimuth/altitude) for a UTC time and observer location
- Observer-to-Sun ENU position vector, distance, and unit direction at any UTC time
- Shared ECEF-to-observer vector conversion for future celestial-object providers
- Daily solar events in UTC (solar noon and sunrise/sunset when available)
- Day-of-year (DOY) shown in the printed UTC timestamp line

## Quick Start

Build debug (default):

```bash
./build.sh
```

Build release:

```bash
./build.sh release
```

Build debug and run all tests:

```bash
./build.sh test
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
./bin/astro --read names.ini --name ny_aalesund
```

Name edit mode (updates `names.ini` by default):

```bash
./bin/astro --name ny_aalesund --lat 78.12345678 --lon 11.98765432 --alt 8.2
```

If the name exists, the program asks for confirmation, comments out the old line,
and appends the new definition.

Latitude and longitude are decimal degrees. `--alt` is height in metres above the
WGS-84 ellipsoid, not height above sea level or ground; its default is zero.
The current solar-event calculation does not apply observer height or terrain.
See [coordinate conventions](docs/COORDINATES.md).

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
./bin/astro --read <filename>.ini
```

## Selecting objects

Use `--objects` with a comma-separated list of `sun` and `moon`:

```bash
./bin/astro -r ny_aalesund.ini --objects moon
./bin/astro -r ny_aalesund.ini --objects sun,moon
./bin/astro -r ny_aalesund.ini --objects moon --tui
```

Without `--objects`, text output shows the Sun and TUI shows both objects.
Explicit selection applies to either display mode. Each selected object has
labelled angles and observer distance; daily solar events appear only when the
Sun is selected. Moon output also includes phase name and illuminated percentage.
Lunar rise/set times are not yet available. Objects display in
Sun/Moon order regardless of list order; duplicate names are ignored.
Names are lowercase; empty entries and unsupported names are errors.
Specify `--objects` once. It cannot be used in name-edit mode.

## Interactive sky chart

With ncurses development files installed at build time:

```bash
./bin/astro --tui -r ny_aalesund.ini --date 2026-02-25 --time 08:56:07
```

Use left/right arrows to step one minute, down/up to step one hour, `r` to
restore the starting timestamp, and `q` to quit. Time is UTC and stays fixed
between key presses. The ASCII sky chart has north at top, east at right,
the horizon at the rim and zenith at the centre. `*` is yellow on colour terminals and marks the Sun only when
its centre is above the geometric horizon. A separate ±5° altitude view and
signed numeric altitude show below-horizon positions. The zoom is a vertical
altitude scale at the Sun's current azimuth; positions outside its range are
labelled. No terrain or atmospheric refraction is applied to the chart.

The Moon appears as a white `o`, with its own azimuth, signed altitude and
above/below-horizon label. Both bodies follow the same timestamp and time controls.
Only above-horizon centres appear in the sky disc; `@` means both occupy the same
terminal cell. The horizon zoom uses separate Sun and Moon columns (altitude only,
not angular separation). No marker is drawn there for a body outside ±5°.
Lunar angles include observer parallax and height using an approximate orbital
model; see [lunar model limitations](docs/API.md#moon-position).

The Moon panel includes its phase name, illuminated percentage and a small
schematic disc (`#` lit, `.` dark). Waxing is lit on the right, waning on the
left; this is not rotated to match the observer's sky. The phase remains visible
below the horizon and updates with time stepping. The phase diagram is separate
from the `o` sky marker. New/quarter/full names use ±5° bands in the Moon-minus-Sun
ecliptic longitude (lower bound included, upper excluded); intermediate names
fill the remaining intervals. These are display categories, not exact event times.

The terminal must be at least 76 columns by 36 rows; smaller windows show a
resize prompt. Daily event times retain UTC day offsets relative to the displayed
date. Events use the existing standard −0.833° threshold, so sunrise can occur
while the Sun's centre is below the chart's geometric horizon.

TUI support is optional: builds without ncurses retain the normal CLI and
report an error for `--tui`. See [build options](docs/BUILD.md).

## Documentation

Detailed docs are in the docs folder:

- [docs/BUILD.md](docs/BUILD.md): build and run workflows
- [docs/API.md](docs/API.md): public API surface and usage examples
- [docs/COORDINATES.md](docs/COORDINATES.md): WGS-84, ECEF, height references, and local horizon geometry
- [docs/PROJECT_STRUCTURE.md](docs/PROJECT_STRUCTURE.md): repository layout and responsibilities
- [docs/GIT_WORKFLOW.md](docs/GIT_WORKFLOW.md): git helper commands and common workflows
- [docs/DOXYGEN.md](docs/DOXYGEN.md): generate browsable API documentation

## Requirements

- CMake >= 3.21 for `build.sh`/presets; >= 3.16 for direct CMake configuration
- A C++17 compiler (current presets use g++)

## License

See LICENSE.
