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
