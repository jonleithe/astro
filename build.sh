#!/usr/bin/env bash
set -euo pipefail

usage() {
    echo "Usage: $0 [debug|release|docs|install]"
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
    install)
        completion_dir="${HOME}/.local/share/bash_completion/completions"
        completion_src="$(dirname "$0")/scripts/astro-completion.bash"
        completion_dst="${completion_dir}/astro"

        if [[ ! -f "$completion_src" ]]; then
            echo "Error: completion script not found at ${completion_src}"
            exit 1
        fi

        bashrc_file="${HOME}/.bashrc"
        source_line="source ${completion_src}"

        mkdir -p "$completion_dir"
        cp "$completion_src" "$completion_dst"
        echo "Bash completion installed to: ${completion_dst}"

        if ! grep -qF "$source_line" "$bashrc_file" 2>/dev/null; then
            printf '\n# astro tab completion\n%s\n' "$source_line" >> "$bashrc_file"
            echo "Added source line to: ${bashrc_file}"
        else
            echo "Source line already present in: ${bashrc_file}"
        fi

        echo "Open a new shell (or run: source ~/.bashrc) for completion to take effect."
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
