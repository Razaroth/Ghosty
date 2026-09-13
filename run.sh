#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

JOBS="${JOBS:-$(nproc)}"

build() {
    qmake6 opacity-slider.pro
    make -j"$JOBS"
}

install_autostart() {
    local bin
    bin="$(pwd)/opacity-slider"
    mkdir -p "$HOME/.config/autostart"
    cat > "$HOME/.config/autostart/opacity-slider.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Ghosty
Comment=Per-window transparency for KDE
Exec=$bin
Icon=transparency
Terminal=false
X-KDE-autostart-after=panel
EOF
    echo "Autostart installed: ~/.config/autostart/opacity-slider.desktop"
}

case "${1:-}" in
    --install)
        build
        install_autostart
        exec ./opacity-slider
        ;;
    *)
        build
        exec ./opacity-slider "$@"
        ;;
esac