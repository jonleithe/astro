# Working on astro

This repository is a C++17 geodesy and solar-position CLI. Read
[README.md](README.md) for usage and [docs/HANDOFF.md](docs/HANDOFF.md) for the
latest work and proposed next steps. Check the working tree and recent commits
before editing; preserve existing user changes.

## Build and verification

- `./build.sh` builds debug; `./build.sh release` builds release.
- `./build.sh test` builds debug and runs the solar/vector and geodesy tests.
- `./bin/astro --help` displays CLI usage.
- Reproducible solar example:
  `./bin/astro -r ny_aalesund.ini --date 2026-02-25 --time 08:56:07`.
- Run relevant checks for code changes and `git diff --check` before committing.
  Documentation-only changes do not need a rebuild.
- Generated `build/`, `bin/`, and local legacy `astro/` and `astro2/` directories
  are ignored; work in the active source tree.

## Code responsibilities and conventions

- `main.cpp`: orchestration; keep calculations and presentation in their modules.
- `inc/` and `src/`: declarations and implementations, respectively.
- `geo_point`: WGS-84 geodetic/ECEF conversion and observer geometry.
- `observer_vector.h`: shared east/north/up vectors and normalization.
- `sun_position`: solar position, daily events, and observer-to-Sun vectors.
- `cli_input` and `cli_file_io`: arguments, configuration, and location files.
- `print_output`: presentation; usage text is defined in `inc/print_output.h`
  and printed using `std::cout`. Large output values use spaces between groups
  of three digits, with a decimal point and existing precision.
- Follow surrounding C++ style and keep dependencies proportionate to the task.
- Times are UTC. Angles are degrees at public interfaces; position vectors are
  metres. ENU means east, north, up. Observer height is ellipsoidal height.
- Read `docs/COORDINATES.md` and `docs/API.md` before changing numerical behavior;
  preserve the documented model limitations.
- Update relevant documentation when changing public behavior. Keep handoff
  notes factual and distinguish proposed work from implemented features.

## Continuing across machines

Keep repository guidance and handoff notes portable: use repository-relative
paths, with no machine-specific configuration or credentials. Commit and push
when requested; report checks performed and any remaining local changes.
