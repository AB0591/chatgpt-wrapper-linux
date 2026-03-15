#!/usr/bin/env bash

set -euo pipefail

repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
binary_source="$repo_dir/target/release/chatgpt-wrapper"
install_dir="${HOME}/.local/bin"
binary_target="${install_dir}/chatgpt-wrapper"
desktop_dir="${HOME}/.local/share/applications"
desktop_file="${desktop_dir}/chatgpt-wrapper.desktop"

if [[ ! -x "$binary_source" ]]; then
    echo "Missing release binary at $binary_source"
    echo "Build it first with: cargo build --release"
    exit 1
fi

mkdir -p "$install_dir" "$desktop_dir"
install -m 0755 "$binary_source" "$binary_target"

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
echo "Installed launcher to $desktop_file"
