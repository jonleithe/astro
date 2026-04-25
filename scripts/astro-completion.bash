# Bash completion for astro CLI.
#
# Installed automatically by: ./build.sh install
# The installer adds a source line to ~/.bashrc so this is loaded on every
# new shell without manual setup.
#
# Manual one-time load:
#   source scripts/astro-completion.bash

# Absolute path to this script's directory, resolved at source time.
_ASTRO_COMPLETION_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

_astro_completion()
{
    local cur prev
    local -a options

    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD - 1]}"

    options=(
        --read -r
        --name -n
        --lat -a
        --lon -o
        --alt -z
        --date -d
        --time -t
        --help -h
    )

    case "$prev" in
        --read|-r)
            COMPREPLY=( $(compgen -f -- "$cur") )
            return 0
            ;;
        --name|-n)
            COMPREPLY=()
            return 0
            ;;
        --lat|-a|--lon|-o|--alt|-z|--date|-d|--time|-t)
            COMPREPLY=()
            return 0
            ;;
    esac

    if [[ "$cur" == -* ]]; then
        COMPREPLY=( $(compgen -W "${options[*]}" -- "$cur") )
    else
        COMPREPLY=()
    fi
}

complete -F _astro_completion astro
complete -F _astro_completion ./bin/astro
complete -F _astro_completion bin/astro
complete -F _astro_completion "${_ASTRO_COMPLETION_DIR}/../bin/astro"
