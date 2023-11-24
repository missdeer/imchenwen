# imchenwen
[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2Fcoslyk%2Fimchenwen.svg?type=shield)](https://app.fossa.com/projects/git%2Bgithub.com%2Fcoslyk%2Fimchenwen?ref=badge_shield)


imchenwen is an interesting player that lets you to enjoy videos. It can play the video online, download it or just open the local videos.

***

## Homepage

The homepage of imchenwen is here: https://www.dfordsoft.com/imchenwen.html

Here is the development page of this project. For the introduction and usage information, please visit the homepage.

[Update log](https://github.com/missdeer/imchenwen/blob/develop/NEWS.md)

## Installation

#### Windows / macOS / Linux (AppImage)

Download from [GitHub Releases](https://github.com/missdeer/imchenwen/releases) and install it.

**Note:**
1. Windows version only support x64.
2. AppImage version does not support hardware decoding.

#### Linux (Flatpak)

Install from [Flathub](https://flathub.org/apps/details/com.github.coslyk.imchenwen): `flatpak install flathub com.github.coslyk.imchenwen`

#### Linux (Debian)

Add [DebianOpt](https://github.com/missdeer/debianopt-repo) repository, then install with `sudo apt install imchenwen`.

#### Linux (ArchLinux, Manjaro)

Add [ArchLinuxCN](https://www.archlinuxcn.org/archlinux-cn-repo-and-mirror/) repository, then install with `pacman -S imchenwen`.

## Screenshot

![](https://www.dfordsoft.com/files/imchenwen-play.png)

## Development

Following packages are essential for compiling imchenwen.

On ArchLinux:

```
    - cmake
    - ffmpeg
    - mpv
    - python
    - qt6-base
    - qt6-declarative
    - qt6-tools
    - wget / curl
```

On Debian:

```
For building:
    - build-essential
    - cmake
    - cmake
    - qt6-base-dev
    - qt6-base-private-dev
    - qt6-declarative-dev
    - qt6-declarative-private-dev
    - qt6-tools-dev
    - libmpv-dev
    - libcurl4-openssl-dev
    - libssl-dev
For running:
    - ffmpeg
    - qml6-module-qt-labs-settings
    - qml6-module-qtqml-workerscript
    - qml6-module-qtquick
    - qml6-module-qtquick-controls
    - qml6-module-qtquick-dialogs
    - qml6-module-qtquick-layouts
    - qml6-module-qtquick-templates
    - qml6-module-qtquick-window
    - wget / curl
```

Other Linux: Please diy.

Download the source code, then run:

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```

**Note:** imchenwen uses Qt's private API, so you may need to re-compile imchenwen after Qt is upgraded.

## Technology stack

- [Qt](https://www.qt.io/) (License: LGPL-3)

- [libmpv](https://mpv.io/) (License: GPLv2+)

- [ffmpeg](https://ffmpeg.org/) (License: GPLv2+)

- [yk-dlp](https://github.com/yt-dlp/yt-dlp) (License: Unlicense)

- [lux](https://github.com/iawia002/lux) (License: MIT)

- [hlsdl](https://github.com/selsta/hlsdl) (License: MIT)

- [Danmaku2Ass-Cpp](https://github.com/missdeer/danmaku2ass_cpp) (License: WTFPL)

## License

[GPLv3+](https://github.com/missdeer/imchenwen/blob/develop/LICENSE)

[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2Fcoslyk%2Fimchenwen.svg?type=large)](https://app.fossa.com/projects/git%2Bgithub.com%2Fcoslyk%2Fimchenwen?ref=badge_large)