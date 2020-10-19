# VTerm
A terminal emulator that makes your life easier.

**Note:** We are still in beta version. Core features are pretty stable, more
work is still needed on code clean-up and documentation file.

## Features
- **Highly customizable:** an easy to write configuration file with various
  options with sane defaults.
- **Advanced Tabs support:**
    - Smart tabs shortcuts e.g. alternate tab, numbered tabs.
    - Configurable tab showing policy.
    - Configurable new tab placement.
    - Configurable new tab directory.
- **Style theming:** use CSS to style your tab bar and search prompts or leave
  it to system defaults. **TODO add links to examples here**
- **VIM-like keyboard shortcuts:** yiw, yy, w, $, visual line and block modes
  and all the other good stuff, see full list below **TODO Add link here**.
- **Prompt up/down:** ever got lost in a long command output trying to read the
  previous command? No more! Jump to previous/next prompt directly.
- **Clickable links:** see a link or a `mailto`? Just click it!
- **Focus-aware background:** configurable background color and transparency
  based on terminal focus status so it is easy to identify currently focused
  terminal.
- **Bidirectional text support:** no more messed-up text that is written in
  right-to-left languages.
- **Well commented & structured code:** want to contribute? Feeling an urge to
  hack your own terminal? Take a look at the code!
- **Lots more:** fullscreen, zoom-in/zoom-out, support for terminal images
  (using Überzug **TODO Add link**).

## Getting Started
### Quick Installation
#### Ubuntu [20.04, 18.04]
Select all and paste in your terminal:
```bash
(
# Install dependencies
sudo apt-get install -y cmake pkg-config libgtk-3-dev libpcre2-dev \
    valac gtk-doc-tools intltool libglib3.0-cil-dev libgnutls28-dev \
    libgirepository1.0-dev libxml2-utils gperf build-essential libsystemd-dev;

# Install meson (https://mesonbuild.com/Quick-guide.html)
sudo apt-get install -y python3 python3-pip python3-setuptools python3-wheel ninja-build;
pip3 install --user meson;

# make sure meson bin is in PATH
[ -x "$(command -v meson)" ] || export PATH="~/.local/bin:$PATH";

# clone VTerm
git clone --recursive https://github.com/man9ourah/vterm.git;
cd vterm;

# Ubuntu 20.04 have the new fribidi in apt
if [ "$(lsb_release -sr)" == "20.04" ]; then
    sudo apt-get install -y libfribidi-dev

    # Build & install VTerm without rpath set
    meson build && cd build && sudo ninja install;
else
    # Install fribidi
    git clone https://github.com/fribidi/fribidi .deps/fribidi;
    pushd .deps/fribidi;
        meson -Ddocs=false build;
        cd build;
        sudo ninja install;
    popd;

    # Build & install VTerm with rpath set
    meson -Dset_install_rpath=true build && cd build && sudo ninja install;
fi

)
```

#### Debian
TODO
#### Fedora
TODO
#### Arch
TODO

### Dependencies
- meson >= 0.50.0 [Install](https://mesonbuild.com/Quick-guide.html).
- ninja [Install](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages).
- pkg-config [Install](https://www.freedesktop.org/wiki/Software/pkg-config/).
- cmake [Install](https://cmake.org/install/).
- GTK3 [Install](https://www.gtk.org/docs/installations/linux/).
- GNU Fribidi [Install](https://github.com/fribidi/fribidi).
- PCRE2 [Install](https://sourceforge.net/projects/pcre/).
- GNUTLS [Install](https://www.gnutls.org/download.html).
- Systemd [Install](https://www.freedesktop.org/wiki/Software/systemd/).

**You think this list is incomplete?** Please make a PR or open an issue.

### Building & Installing
```bash
meson build && cd build && sudo ninja install
```

### Shell Integration
TODO:: Explain shell integration for prompts and cwd.

### Configuration
TODO:: Add configuration information and examples.

TODO:: Link a wiki page with detailed configuration options information.

### Styling
TODO:: Add information about notebook & search box CSS file.

### Commands and shortcuts
TODO:: Add a table for keyboard shortcuts and commands.

## Contribution
TODO:: Add information for how to contribute (todo.md)


