# AE1200 Strata

Open-source software for a custom Casio AE-1200 smart module featuring a
memory-in-pixel (MIP) display.

## Native V1 display app

The native simulator and future firmware share a portable C framebuffer
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
opening currently reels between the classic indicators and an animated battery
level every four seconds. The main opening alternates between the classic clock
and a notification card with a scrolling message. See
[packaging/README.md](packaging/README.md) for removal and details.

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

The custom Zephyr driver, ItsyBitsy overlay, power sequence, EXTCOMIN service,
and UF2 instructions are in [firmware/README.md](firmware/README.md). From an
nRF Connect SDK terminal, build the exact emulator framebuffer with:

```sh
make firmware
```
