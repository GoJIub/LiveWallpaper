#!/bin/bash
set -e

if [[ "$1" == "--enable" ]]; then
    systemctl --user enable livewallpaper
    systemctl --user start livewallpaper
    echo "LiveWallpaper enabled."
    exit 0
fi

echo "Building LiveWallpaper..."
cmake -B build
cmake --build build

echo "Installing LiveWallpaper..."
sudo cmake --install build

echo "Setting up autostart..."
mkdir -p ~/.config/systemd/user
cp /usr/local/share/livewallpaper/livewallpaper.service ~/.config/systemd/user/
systemctl --user daemon-reload
systemctl --user enable livewallpaper
systemctl --user start livewallpaper

systemctl --user start livewallpaper
echo "Done! LiveWallpaper has been installed and started."