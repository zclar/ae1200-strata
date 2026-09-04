# Architecture and hardware notes

## Goal

V1 is a non-interactive product demonstration. The current reference scene is
the classic AE-1200 face; additional layouts can be added after the physical
mask is calibrated. The desktop simulator is the fast feedback loop; firmware
and display transport are separate targets.

## Proposed layers

1. **Demo model** — scene order, timing, and synthetic data, written in portable C.
2. **Watch UI** — portable C drawing code that produces a fixed 176 x 176 RGB111
   framebuffer. This is the single visual source of truth.
3. **Display backend** — native Tk during visual development; the in-tree
   LPM013M126A SPI transport on Zephyr hardware.
4. **Board configuration** — Devicetree overlays for nRF52840 development hardware
   and nRF54L15 production hardware.

The initial browser prototype intentionally has no package dependencies and is
for rapid UX exploration. It is not the accuracy reference. Before hardware
validation, its scene direction will move into the shared C renderer, compiled
for both the simulator and Zephyr. No layout should be maintained independently
in JavaScript and firmware.

## Simulator-to-hardware parity

For a given scene ID, timestamp, and synthetic input, all targets must produce
the same packed RGB111 framebuffer. Automated tests will hash golden framebuffers
and compare simulator output with firmware-generated captures. This verifies
pixel content, clipping, mask-safe regions, animation timing, and color indices.

The desktop cannot reproduce reflective optics, viewing angle, ambient-light
behavior, or the physical faceplate shadow. Those require real-panel photography
under controlled lighting. Simulator colors should therefore represent the
eight logical RGB111 states, while an optional visual profile approximates the
panel's appearance without changing framebuffer data.

## Animation compositor

Animation is organized as small, fixed-memory reels in the portable C renderer.
Each reel is a static array of renderer callbacks; the compositor selects one
callback and gives it a local timeline from 0 through 3999 ms. Every item lasts
exactly four seconds. The top status and main display openings can therefore
grow independently by adding a callback and one array entry.

This is intentionally not containerized. A process or container boundary would
not exist on the nRF52840/nRF54L15 and would break simulator-to-hardware parity.
The callback compositor has no heap allocation, draws directly into the shared
176 x 176 RGB111 framebuffer, and compiles unchanged for the native app and
Zephyr. As the library grows, renderer callbacks can move into separate widget
source files without changing the timing or backend interfaces.

The native app requests 10 ms presentation steps for visibly smooth previews,
but animation state always comes from an absolute monotonic clock. A slow host
therefore skips an intermediate frame instead of stretching the four-second
timeline. The physical panel remains transport-limited to its documented update
rate, so hardware may display fewer intermediate frames while following the
same elapsed time and arriving at the same four-second boundaries.

## Confirmed display constraints

- JDI LPM013M126A
- AE-1200 case envelope: approximately 45.0 x 42.1 x 12.5 mm
- 176 x 176 addressable pixels
- RGB111: one bit each for red, green, and blue (eight colors total)
- Active image area: 23.0208 x 23.0208 mm
- Approximate module outline: 26.02 x 27.82 x 0.974 mm
- Three-wire serial interface with line-addressable updates
- Maximum image update rate: approximately 10 frames per second
- Reflective panel with no backlight

The simulator renders only colors available in RGB111 and uses integer scaling
to preserve the real pixel grid.

## Faceplate calibration

The included mask is a provisional visualization, not manufacturing geometry.
Exact layout requires one of:

- A dimensioned CAD drawing of the replacement top cover; or
- A flatbed scan/straight-on macro photograph with a ruler in the same plane,
  plus at least the inside width and height measured with calipers.

Useful measurements are the outer display opening, each divider's width and
centerline, corner radii, and the panel active area's offset relative to fixed
case features. These values should become a versioned mask geometry file so the
same safe regions drive the simulator, UI layout checks, and mechanical CAD.

## Firmware direction

Use Nordic nRF Connect SDK/Zephyr rather than the legacy nRF5 SDK. The same app
can be built for an nRF52840 development board and later for
`nrf54l15dk/nrf54l15/cpuapp`, with board-specific pins and peripherals confined
to Devicetree overlays. The custom transport in `firmware/src/lpm013m126a.c`
owns SPI framing, line updates, display enable, and COM polarity timing.

Do not connect the bare panel until the exact panel revision's electrical and
timing tables, FPC pinout, supply rails, and level requirements have been checked
against the carrier board.

The development target is the Adafruit ItsyBitsy nRF52840 Express using its UF2
bootloader. Its reserved pins and power rails are captured in the board-specific
Devicetree overlay and `docs/hardware-wiring.md`; the remaining validation step
is continuity-checking the passive FPC adapter before applying power.
