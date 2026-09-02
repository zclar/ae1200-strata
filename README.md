# AE1200 Strata

Open-source software for a custom Casio AE-1200 smart module featuring a
memory-in-pixel (MIP) display.

## V1 display simulator

The browser simulator models the JDI LPM013M126A's 176 x 176 pixel, eight-color
display and automatically cycles through product-demo layouts.

```sh
python3 -m http.server 8000
```

Open <http://localhost:8000/simulator/>. No package installation or build step
is required. Use the controls to pause, advance scenes, toggle the faceplate
mask, or show the pixel grid.

See [docs/architecture.md](docs/architecture.md) for the firmware plan and
known hardware constraints.
