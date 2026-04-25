# Build and Run

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

The binary is written to bin/astro:

```bash
./bin/astro
```

## VS Code

The workspace includes tasks and a launch configuration for debug builds.

- Prelaunch build task: Build (Debug)
- Launch target: bin/astro
