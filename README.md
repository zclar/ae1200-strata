# AE1200 Strata

Open-source software for a custom Casio AE-1200 smart module featuring a
memory-in-pixel (MIP) display.

## Native V1 display simulator

The native simulator and future firmware share a portable C framebuffer
renderer. It requires GCC, Python 3, and Tk 8.6, with no third-party packages.

```sh
make simulator
```

## Browser UX prototype

The earlier browser simulator remains available for convenient UX experiments.
The native shared-C simulator is the pixel-accuracy reference.

```sh
python3 -m http.server 8000
```

Open <http://localhost:8000/simulator/>. No package installation or build step
is required. Use the controls to pause, advance scenes, toggle the faceplate
mask, or show the pixel grid.

See [docs/architecture.md](docs/architecture.md) for the firmware plan and
known hardware constraints.
