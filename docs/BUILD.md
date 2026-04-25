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

This runs:

- Configure preset: release-gcc
- Build preset: build-release

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
./bin/astro --read locations.ini.example --name ny_aalesund
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

Minimal example:

```bash
./bin/astro --lat 78.24490 --lon 15.49369
```

Read from file:

```bash
./bin/astro --read astro.conf.example
```

## Tests

Run the solar sanity tests:

```bash
ctest --test-dir build/debug --output-on-failure
```

## VS Code

The workspace includes tasks and a launch configuration for debug builds.

- Prelaunch build task: Build (Debug)
- Launch target: bin/astro
