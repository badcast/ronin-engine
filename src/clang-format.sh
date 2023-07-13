#!/usr/bin/sh

# Format sources with Clang-Format using ../.clang-format configuration

_script_path=$(dirname "$(readlink -f "$0")")

function check_util() {
    if [ ! -f "$(command -v "$1")" ]; then
        echo "Utility \"$1\" is not installed"
        exit 1
    fi
}

function format_file() {
    find "$1" -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) -exec sh -c 'truncate -s 0 /tmp/_______format && clang-format --assume-filename=../.clang-format {} | tee /tmp/_______format > /dev/null && mv /tmp/_______format {}' \;
}

check_util clang-format
check_util find
check_util tee

format_file "${_script_path}"
format_file "${_script_path}/../include"
