# DLNA Browser

A desktop DLNA/UPnP media browser built in C++20 with Qt 6. Discovers media servers on the local network and lets you browse and play their content directly from the application.

---

## Features

### Server discovery
- Automatic LAN scanning via SSDP (UPnP M-SEARCH)
- Server list with icons fetched from each device's descriptor

### Content browsing
- Full folder hierarchy navigation
- Breadcrumb address bar with back / forward / up / home history
- List view and icon view, toggled from the toolbar
- Icon size slider in the status bar
- Sort by name or date (ascending/descending), saved per folder and inherited by subfolders
- Favorites panel for bookmarking locations

### Media playback
- Dedicated `MediaViewer` window opened on item activation
- Video, audio, image, and file-info views
- Playback overlay with play/pause, mute, seek bar, and elapsed/total time
- Audio mode with album art circle
- Previous/Next navigation: status bar buttons, `PageUp`/`PageDown`, `⌘←` / `⌘→`
- `Esc` closes the viewer; `Space`/`Enter` toggles pause; `←`/`→` seeks ±5 s
- Main window selection tracks the currently open file in the viewer

### Persistent settings
- List/icon view mode saved across sessions
- Icon size saved across sessions
- Sort order saved per folder

---

## Dependencies

| Dependency | Version | Notes |
|------------|---------|-------|
| **Qt 6** | ≥ 6.4 | Core, Gui, Widgets, Network, Xml, Multimedia, MultimediaWidgets |
| **CMake** | ≥ 3.20 | Build system |
| **C++ compiler** | C++20 | Clang (macOS), GCC, or MSVC |

Qt 6 is the only external dependency. The **Font Awesome 6 Solid** typeface is embedded in the binary via Qt Resource System (`resources/resources.qrc`) — no system fonts required.

### Installing Qt 6

**macOS (Homebrew)**
```bash
brew install qt
```

**Ubuntu / Debian**
```bash
sudo apt install qt6-base-dev qt6-multimedia-dev libqt6xml6-dev
```

---

## Building

### macOS — Apple Silicon

```bash
git clone <repo-url> dlna_browser
cd dlna_browser
make
```

The Makefile passes `-DCMAKE_OSX_ARCHITECTURES=arm64` to CMake automatically. The resulting app bundle is at `build/DlnaBrowser.app`.

### macOS — Intel / Linux

```bash
cmake -S . -B build
cmake --build build
```

### Windows

```bat
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

### Makefile targets

| Target | Description |
|--------|-------------|
| `make` / `make build` | Configure (if needed) and build the application |
| `make test` | Build everything and run unit tests |
| `make clean` | Remove build artefacts |

---

## Tests

Unit tests cover XML parsing (DIDL-Lite, SOAP responses, device descriptors) and helper functions. They require only Qt 6 Test — no network or GUI event loop needed.

```bash
make test
# or directly:
ctest --test-dir build --output-on-failure
```

---

## Architecture

```
src/
├── main.cpp              # Entry point, QApplication setup
├── mainwindow.*          # Main window, navigation, history
├── dlnadiscovery.*       # SSDP server discovery
├── dlnaclient.*          # ContentDirectory SOAP requests
├── dlnaparser.*          # Pure XML parsing (DIDL-Lite, descriptors) — unit-tested
├── dlnamodel.*           # QAbstractListModel for folder contents
├── dlnaitem.h            # Data types: DlnaItem, DlnaLocation
├── contentview.*         # List/icon view widget with mode switching
├── mediaviewer.*         # Playback window (QMainWindow)
├── videowidget.*         # Video/audio player (QVideoSink-based)
├── imagewidget.*         # Image viewer with zoom and pan
├── infowidget.*          # File info card
├── addressbar.*          # Breadcrumb bar
├── favoritespanel.*      # Favorites panel
└── faicon.*              # Font Awesome icons rendered from TTF
tests/
└── test_dlnaparser.cpp   # Qt Test unit tests (30 cases)
```

### Notable design decisions

- **QVideoSink instead of QVideoWidget** — avoids the macOS Metal/Qt layer conflict; frames are rendered manually in `paintEvent()`, allowing the control overlay to sit on top without native-layer issues.
- **DlnaParser as pure functions** — all XML parsing logic is extracted into a stateless `DlnaParser` namespace, making it testable without a network connection or Qt event loop.
- **Font Awesome embedded in QRC** — icons have no dependency on installed system fonts; the TTF file is compiled into the binary.

### AI Notice

This project was built with significant assistance from AI. The architecture, source code, and tests were generated and iteratively developed in collaboration with an AI assistant.

