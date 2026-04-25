#!/usr/bin/env bash
set -euo pipefail

usage() {
    echo "Usage: $0 [debug|release|docs]"
    exit 1
}

if [[ $# -gt 1 ]]; then
    usage
fi

mode="${1:-debug}"

case "$mode" in
    debug)
        configure_preset="debug-gcc"
        build_preset="build-debug"
        ;;
    release)
        configure_preset="release-gcc"
        build_preset="build-release"
        ;;
    docs)
        if ! command -v doxygen >/dev/null 2>&1; then
            echo "Error: doxygen is not installed or not in PATH."
            echo "Install doxygen and run: $0 docs"
            exit 1
        fi

        if [[ ! -f "Doxyfile" ]]; then
            echo "Error: Doxyfile not found in project root."
            exit 1
        fi

        echo "Generating API documentation with Doxygen..."
        doxygen Doxyfile
        echo "Documentation generated: docs/doxygen/html/index.html"
        exit 0
        ;;
    *)
        usage
        ;;
esac

echo "Configuring with preset: ${configure_preset}"
cmake --preset "${configure_preset}"

echo "Building with preset: ${build_preset}"
cmake --build --preset "${build_preset}"

echo "Build completed (${mode})."
