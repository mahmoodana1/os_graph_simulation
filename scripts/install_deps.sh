#!/usr/bin/env bash
# Install build dependencies for os_graph_simulation.
# Supports Arch Linux and Ubuntu/Debian. Installs gcc, make, git,
# raylib 5.5 (built from source on Ubuntu since apt ships an older version)
# and the X11/GL libraries raylib needs at link time.

set -euo pipefail

RAYLIB_VERSION="5.5"

log()  { printf '\033[1;32m[deps]\033[0m %s\n' "$*"; }
warn() { printf '\033[1;33m[deps]\033[0m %s\n' "$*" >&2; }
die()  { printf '\033[1;31m[deps]\033[0m %s\n' "$*" >&2; exit 1; }

require_cmd() {
    command -v "$1" >/dev/null 2>&1 || die "missing required command: $1"
}

detect_distro() {
    if [[ -r /etc/os-release ]]; then
        # shellcheck disable=SC1091
        . /etc/os-release
        echo "${ID:-unknown}"
    else
        echo "unknown"
    fi
}

sudo_run() {
    if [[ $EUID -eq 0 ]]; then
        "$@"
    else
        require_cmd sudo
        sudo "$@"
    fi
}

install_arch() {
    log "Installing packages via pacman..."
    sudo_run pacman -Sy --needed --noconfirm \
        base-devel gcc make git pkgconf cmake \
        libx11 libxrandr libxinerama libxcursor libxi mesa

    if pacman -Qq raylib >/dev/null 2>&1; then
        warn "Removing pacman 'raylib' package — repo ships a newer version than $RAYLIB_VERSION."
        sudo_run pacman -Rns --noconfirm raylib || true
    fi

    ensure_raylib
}

install_ubuntu() {
    log "Installing packages via apt..."
    sudo_run apt-get update
    sudo_run apt-get install -y --no-install-recommends \
        build-essential gcc make git pkg-config cmake ca-certificates wget \
        libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
        libgl1-mesa-dev libasound2-dev

    ensure_raylib
}

ensure_raylib() {
    if pkg-config --exists raylib 2>/dev/null; then
        local installed
        installed="$(pkg-config --modversion raylib)"
        if [[ "$installed" == "$RAYLIB_VERSION" ]]; then
            log "raylib $installed already installed."
            return
        fi
        warn "raylib $installed present, but project requires exactly $RAYLIB_VERSION."
    fi
    build_raylib_from_source
}

build_raylib_from_source() {
    log "Building raylib $RAYLIB_VERSION from source..."
    local tmpdir
    tmpdir="$(mktemp -d)"
    trap 'rm -rf "$tmpdir"' RETURN

    git clone --depth 1 --branch "$RAYLIB_VERSION" \
        https://github.com/raysan5/raylib.git "$tmpdir/raylib"

    cmake -S "$tmpdir/raylib" -B "$tmpdir/build" \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=ON \
        -DCMAKE_INSTALL_PREFIX=/usr/local

    cmake --build "$tmpdir/build" -j"$(nproc)"
    sudo_run cmake --install "$tmpdir/build"
    sudo_run ldconfig
    log "raylib $RAYLIB_VERSION installed to /usr/local."
}

main() {
    local distro
    distro="$(detect_distro)"
    log "Detected distribution: $distro"

    case "$distro" in
        arch|manjaro|endeavouros|cachyos)
            install_arch
            ;;
        ubuntu|debian|pop|linuxmint)
            install_ubuntu
            ;;
        *)
            die "Unsupported distribution: $distro (supported: arch, ubuntu)"
            ;;
    esac

    log "All dependencies installed. Run 'make milestone2' to build."
}

main "$@"
