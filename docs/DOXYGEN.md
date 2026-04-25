# Doxygen

[Back to README](../README.md)

Related docs:

- [Build and run](BUILD.md)
- [API reference](API.md)
- [Project structure](PROJECT_STRUCTURE.md)

This project includes a Doxygen config in the repository root:

- Doxyfile

## Generate API docs

From project root:

```bash
./build.sh docs
```

Or run Doxygen directly:

```bash
doxygen Doxyfile
```

Generated HTML entry point:

- docs/doxygen/html/index.html

## Notes

- Input paths are currently `inc` and `src`
- Legacy trees (`astro`, `astro2`) are excluded

## CLI Read-File Format

The `astro` binary also supports reading CLI settings from file with:

```bash
./bin/astro --read astro.conf.example
```

File format is simple `key = value` lines:

- Supported keys: `lat`, `lon`, `alt`, `date`, `time`
- Comments start with `#` or `;`
- CLI flags always override values loaded from file
