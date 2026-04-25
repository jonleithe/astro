#!/usr/bin/env bash
set -euo pipefail

usage() {
    echo "Usage: $0 [debug|release]"
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
    *)
        usage
        ;;
esac

echo "Configuring with preset: ${configure_preset}"
cmake --preset "${configure_preset}"

echo "Building with preset: ${build_preset}"
cmake --build --preset "${build_preset}"

echo "Build completed (${mode})."
