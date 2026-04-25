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
- main.cpp: sample executable using the geo API
- inc/geo_point.h: public API declarations
- src/geo_point.cpp: geo implementation
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
