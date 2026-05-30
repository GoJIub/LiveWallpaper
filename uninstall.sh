#!/bin/bash
set -e

if [[ "$1" == "--disable" ]]; then
    systemctl --user stop livewallpaper
    systemctl --user disable livewallpaper
    echo "LiveWallpaper disabled."
    exit 0
fi

if [[ ! -f ~/.config/systemd/user/livewallpaper.service ]]; then
    echo "LiveWallpaper is not installed. Nothing to uninstall."
    exit 0
fi

echo "Stopping LiveWallpaper..."
systemctl --user stop livewallpaper

echo "Disabling LiveWallpaper..."
systemctl --user disable livewallpaper

read -p "Remove user config (~/.config/livewallpaper/)? [y/N] " answer
if [[ "$answer" == "y" || "$answer" == "Y" ]]; then
    rm -rf ~/.config/livewallpaper/
    echo "User config removed."
fi

echo "Removing LiveWallpaper files..."
sudo rm -f /usr/local/bin/LiveWallpaper
sudo rm -rf /usr/local/share/livewallpaper

echo "Removing LiveWallpaper service file..."
rm -f ~/.config/systemd/user/livewallpaper.service

echo "LiveWallpaper has been uninstalled."