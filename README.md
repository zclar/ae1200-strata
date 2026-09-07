# AE1200 Strata

Open-source software for a custom Casio AE-1200 smart module featuring a
memory-in-pixel (MIP) display.

## Native V1 display app

The native simulator and firmware share a portable C framebuffer
renderer. It requires GCC, Python 3, and Tk 8.6, with no third-party packages.

```sh
make app
```

Install **AE1200 Emulator** in the current Linux user's application menu:

```sh
make install-app
```

The app rebuilds the shared renderer automatically when its C source changes.
Use `./bin/ae1200-strata --check` to verify the local runtime without opening a
window. Its faceplate view includes the original-style printed legends, analog
minute track, screws, and exact provisional aperture clipping. The upper status
opening currently reels between the classic indicators and two wide,
eleven-cell LCD battery gauges: one clean bar-only version and one with its
percentage in a dedicated right-side column beside seven uninterrupted cells.
Bars light in discrete steps, with outlined empty cells. The main opening alternates between the
classic clock and fast, smooth-scrolling Messages and Gmail notification cards.
Every reel item lasts
four seconds, with staggered change times for the top status, paired circle/map,
and bottom openings (the first status and circle/map slots are shortened to
establish the stagger). The circle and middle/world-map opening rotate together
through classic, sunny, cloudy, rainy, and thunderstorm cards. The sun restores
the reference-based irregular sprite, reduced from 64x64 to 56x56 pixels. Its
artwork stays static and the whole sun uses the same gentle horizontal drift
as the cloud (one pixel each way over 2.4 seconds). The colored cloud has
overlapping lobes, a continuous black outline, and cyan/blue shadow stipple that
remains visible on the reflective panel.
Rain falls as alternating blue/cyan blocks. The storm card adds two quick
yellow lightning flashes. Tiny three-bar condition meters complete the LCD-style
weather cards. Weather uses Fahrenheit demo temperatures, humidity and UV
readings, plus rain probability on rain/storm cards.
Visual TODO: the sun, cloud, and lightning still need work to look good;
their current artwork is provisional, not a finished visual baseline.
All weather graphics and timing live in the shared C renderer. See
[packaging/README.md](packaging/README.md) for removal and details.

For a visual review with the native palette and faceplate clipping, run
`python3 tests/preview_weather.py` after building the renderer (requires Pillow).
It writes `build/weather-preview.png`, `build/weather-animation.gif`, and
`build/weather-faceplate.png`. These previews come from the shared C output.

## Browser UX prototype

The earlier browser simulator remains available for convenient UX experiments.
The native shared-C simulator is the pixel-accuracy reference.

```sh
python3 -m http.server 8000
```

Open <http://localhost:8000/simulator/>. No package installation or build step
is required. Use the controls to pause, advance scenes, toggle the faceplate
mask, or show the pixel grid.

See [docs/architecture.md](docs/architecture.md) for the firmware plan,
[docs/hardware-wiring.md](docs/hardware-wiring.md) for the reserved development
wiring, and [docs/faceplate-geometry.md](docs/faceplate-geometry.md) for the
current mask evidence and CAD calibration requirements.

## nRF52840 hardware target

First working hardware demo confirmed on **2026-09-06**, using the ItsyBitsy
nRF52840 Express and JDI LPM013M126A. Display SI connects to **MOSI**; leave
**MISO disconnected**. The emulator shows this confirmed baseline while keeping
the faceplate dimensions and reflective color appearance marked as provisional.

The custom Zephyr driver, ItsyBitsy overlay, power sequence, EXTCOMIN service,
and UF2 instructions are in [firmware/README.md](firmware/README.md). From an
environment with nRF Connect SDK v2.9.3 installed, build the same renderer with:

```sh
make firmware
```
