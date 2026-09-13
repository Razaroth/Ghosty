# Ghosty

<img src="logo.png" alt="Ghosty" width="160">

A cute ghost in your system tray that gives KDE Plasma per-window transparency.

## Install

Arch packages are available from the repo, built from the `v1.0.1` tag:

```
curl -L -o ghosty.pkg.tar.zst \
  https://github.com/Razaroth/Ghosty/raw/main/release/ghosty-1.0.1-1-x86_64.pkg.tar.zst
sudo pacman -U ghosty.pkg.tar.zst
```

The KWin effect and script ship inside the package; Ghosty installs itself to
the right places on first launch. Log out and back in (or restart KWin
`kwin_x11 --replace` / `kwin_wayland --replace`) if the transparency doesn't
apply after upgrading.

## From source

Requires Qt 6 (`qt6-base`):

```
qmake6 PREFIX=/usr opacity-slider.pro
make
sudo make install
```

Run directly from a checkout with `./run.sh` (no install needed).

## Building the package

```
make PREFIX=/usr INSTALL_ROOT="$pkgdir" install   # what goes in the package
```

## Usage

- Left-click the tray ghost: transparency slider panel
- Right-click: context menu (panel, clear all overrides, quit)
- Slider applies to the window currently under the mouse

## License

MIT