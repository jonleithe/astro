# Project Structure

[Back to README](../README.md)

Related docs:

- [Build and run](BUILD.md)
- [API reference](API.md)
- [Git workflow](GIT_WORKFLOW.md)

Top-level active project files:

- CMakeLists.txt: build target and compiler settings
- CMakePresets.json: configure/build presets (debug/release)
- build.sh: convenience build entry point
- main.cpp: executable entry point and high-level runtime flow
- inc/geo_point.h: public geodetic API declarations
- inc/sun_position.h: public Sun-position API declarations
- inc/print_output.h: output formatting declarations for CLI usage/results
- inc/cli_file_io.h: config/name-file parsing and update declarations
- inc/cli_input.h: CLI option parsing and merged input resolution declarations
- src/geo_point.cpp: geodetic implementation
- src/sun_position.cpp: Sun-position implementation
- src/print_output.cpp: usage/result printing implementation
- src/cli_file_io.cpp: config and location-library file I/O implementation
- src/cli_input.cpp: CLI parsing and input resolution implementation
- tests/test_sun_position.cpp: lightweight solar calculation sanity tests
- bin/: built executable output
- build/: generated CMake build trees
- scripts/: helper scripts

Additional folders:

- astro/: legacy/original code tree kept in repository
- astro2/: additional historical code tree

When modifying current functionality, prefer working in:

- inc/
- src/
- main.cpp
- CMakeLists.txt
- CMakePresets.json

Module responsibilities in the current CLI executable:

- main.cpp: orchestration only (parse/resolve via modules, compute, print)
- src/cli_input.cpp: option parsing and runtime input resolution
- src/cli_file_io.cpp: --read and --name file handling (read/write)
- src/print_output.cpp: help text and formatted runtime output
