#!/usr/bin/env bash

set -euo pipefail

repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
binary_source="$repo_dir/build/chatgpt-wrapper"
icon_source="$repo_dir/assets/icon.png"
install_dir="${HOME}/.local/bin"
binary_target="${install_dir}/chatgpt-wrapper"
desktop_dir="${HOME}/.local/share/applications"
desktop_file="${desktop_dir}/chatgpt-wrapper.desktop"
icon_dir="${HOME}/.local/share/icons/hicolor/512x512/apps"
icon_target="${icon_dir}/chatgpt-wrapper.png"

if [[ ! -x "$binary_source" ]]; then
    echo "Missing binary at $binary_source"
    echo "Build it first with: make"
    exit 1
fi

if [[ ! -f "$icon_source" ]]; then
    echo "Missing icon at $icon_source"
    exit 1
fi

mkdir -p "$install_dir" "$desktop_dir" "$icon_dir"
install -m 0755 "$binary_source" "$binary_target"
install -m 0644 "$icon_source" "$icon_target"

cat >"$desktop_file" <<EOF
[Desktop Entry]
Type=Application
Name=ChatGPT Wrapper
Comment=ChatGPT desktop wrapper
Exec=${binary_target}
Icon=chatgpt-wrapper
Terminal=false
Categories=Network;
StartupNotify=true
EOF

echo "Installed binary to $binary_target"
echo "Installed icon to $icon_target"
echo "Installed launcher to $desktop_file"
