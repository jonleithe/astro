# Build and Run

[Back to README](../README.md)

Related docs:

- [API reference](API.md)
- [Project structure](PROJECT_STRUCTURE.md)
- [Git workflow](GIT_WORKFLOW.md)
- [Doxygen](DOXYGEN.md)

This project uses CMake presets and a convenience script.

## Build Script

From project root:

```bash
./build.sh
```

This defaults to debug and runs:

- Configure preset: debug-gcc
- Build preset: build-debug

Release build:

```bash
./build.sh release
```

Generate API docs (requires Doxygen):

```bash
./build.sh docs
```

The docs command runs `doxygen Doxyfile`; it does not configure or build C++.
It includes `inc/observer_vector.h` automatically through Doxyfile's header scan.
Doxygen must be installed separately; if missing, the command exits with an error.

Install Bash completion (copies the completion script and updates `~/.bashrc`):

```bash
./build.sh install
```

## Direct CMake Preset Commands

Configure debug:

```bash
cmake --preset debug-gcc
```

Build debug:

```bash
cmake --build --preset build-debug
```

Configure release:

```bash
cmake --preset release-gcc
```

Build release:

```bash
cmake --build --preset build-release
```

## Run

The binary is written to bin/astro and expects observer/time CLI arguments:

```bash
./bin/astro [--read <file>] --lat <deg> --lon <deg> [--alt <m>] [--date YYYY-MM-DD] [--time HH:MM:SS]
```

Name lookup mode:

```bash
./bin/astro --read names.ini --name ny_aalesund
```

Name edit mode (default file is `names.ini`):

```bash
./bin/astro --name ny_aalesund --lat 78.12345678 --lon 11.98765432 --alt 8.2
```

If the name already exists, astro prompts for confirmation, comments out the old
line, and appends the new one.

If date/time are omitted, current UTC date/time are used.

If `--read` is provided, settings are loaded from the file first, then CLI options override them.

If `--name` is provided together with `--read`, file format should be:

```text
name = lat, lon[, alt]
```

Example:

```bash
./bin/astro --lat 78.24490 --lon 15.49369 --alt 0.0 --date 2026-06-21 --time 12:00:00
```

The output includes the observer-to-Sun ENU vector in metres, its dimensionless
unit direction, and observer-to-Sun distance in kilometres. These are calculated
for the supplied UTC timestamp. See the [vector API](API.md#observer-relative-vectors)
for coordinate conventions and approximation limits. No additional dependencies
or CLI flags are needed for vectors.

Minimal example:

```bash
./bin/astro --lat 78.24490 --lon 15.49369
```

Read from file:

```bash
./bin/astro --read astro.conf.example
```

## Tests

Configure and build debug, then run all registered tests:

```bash
./build.sh test
```

The solar suite includes ENU axis orientation, normalization, zero vectors,
seasonal Sun distances, observer height, time evolution, and invalid UTC inputs.
The coordinate suite checks the existing observer-to-target geometry.

To run tests from an existing debug build:

```bash
ctest --test-dir build/debug --output-on-failure
```

## VS Code

The workspace includes tasks and a launch configuration for debug builds.

- Prelaunch build task: Build (Debug)
- Launch target: bin/astro

To set CLI arguments for debugging, edit `.vscode/launch.json` and update the
`args` array in the `cppdbg` launch configuration. Each flag/value is a separate
array element.

Example:

```json
"args": ["--read", "names.ini", "--name", "ny_aalesund"]
```

Another example:

```json
"args": ["--lat", "78.24490", "--lon", "15.49369", "--date", "2026-06-21", "--time", "12:00:00"]
```

For an existing release build, use `ctest --test-dir build/release --output-on-failure`.
Debug and release builds both compile the executable and test targets; only the
`test` script mode also runs tests. The preset workflow requires CMake 3.21 or
newer (preset schema version 3); direct configuration without presets requires
CMake 3.16 or newer.

## Optional terminal interface

CMake detects ncurses automatically. Install its development package before
configuring (for example `libncurses-dev` on Debian/Ubuntu or `ncurses-devel`
on Fedora). The normal CLI also builds without ncurses.

To explicitly disable the TUI, configure with `-DASTRO_ENABLE_TUI=OFF`.
To enable detection again, use `-DASTRO_ENABLE_TUI=ON`.
The TUI uses the same observer/date/time arguments plus `--tui`; see the
[interactive chart instructions](../README.md#interactive-sky-chart).
