# Hardware firmware

This Zephyr application renders the same portable C framebuffer used by the
desktop emulator and writes it to the JDI LPM013M126A in native RGB111 4-bit
transfer mode. The in-tree driver deliberately owns SCS because this panel's
chip select is active high and has explicit setup/hold timing.

## Build for the Adafruit ItsyBitsy nRF52840 Express

The reproducible build uses Nordic nRF Connect SDK **v2.9.3** and its matching
toolchain. Install Nordic nRF Util and its SDK manager, then run once:

```sh
nrfutil install sdk-manager
nrfutil sdk-manager install v2.9.3
```

From the repository root, `make firmware` launches the matching Nordic toolchain
automatically through `bin/build-firmware`. It expects the SDK under `$HOME/ncs`;
set `STRATA_NCS_ROOT` to use another installation directory. Build files are kept
in `build/firmware`, separate from the desktop shared library. The build uses the
board's UF2 application partition and does not build a replacement bootloader.

The portable renderer and JDI packet format are covered by host tests with
`make test`. Compilation verifies the SDK integration; physical display output
still needs visual confirmation on the wired panel.

To flash through the stock UF2 bootloader:

1. Double-tap reset on the ItsyBitsy; the `ITSY840BOOT` drive appears.
2. Copy `build/firmware/zephyr/zephyr.uf2` to that drive.
3. The board restarts and refreshes the animated classic face continuously. At
   the conservative 1 MHz SPI setting, full-frame transfer time keeps the rate
   below the panel's 10 FPS limit; EXTCOMIN continues independently at 1 Hz.

The exact FPC wiring and decoupling are in `docs/hardware-wiring.md`. Do not
connect power until adapter pin order has been checked for reversal with a
multimeter.

## Color and emulator parity

The renderer has exactly eight logical colors: black, blue, green, cyan, red,
magenta, yellow, and white. The demo background is `STRATA_WHITE` (RGB bits
`111`), packed into the panel's `RGB0` transfer nibble as `1110`. It does not
request gray. The desktop's gray-green representation of white is an uncalibrated
approximation of a reflective LCD, not an additional hardware color. Actual
appearance depends on the panel and lighting.

Both targets compile `core/src/strata_display.c` and `core/src/strata_jdi.c`.
Desktop tests validate pixel content and encoded packets; they cannot establish
that a real panel received the data. The firmware prints a status line over USB
every four seconds after successful frame writes, without waiting for a serial
monitor to connect. That confirms software progress, not panel acknowledgement
(the panel has no readback wire).

## Bring-up record — 2026-09-06

- Installed nRF Connect SDK v2.9.3 with toolchain bundle `b77d8c1312`.
- Successfully built the ItsyBitsy application: 46,204 bytes flash, 48,632 bytes
  RAM. UF2 is 92,672 bytes, family `0xADA52840`, starting at `0x26000`.
- Validated every UF2 block stays inside the board's application partition
  (`0x26000` through `0xECFFF`). No bootloader or SoftDevice data is included.
- Confirmed the connected board reports USB `239a:8051` and the generated
  Devicetree selects the reserved SCK/MOSI/D10/D7/D9 pins.
- Uploaded successfully through `ITSY840BOOT` (bootloader 0.9.0, board ID
  `nRF52840-ItsyBitsy-revA`, SoftDevice S140 6.1.1). The board restarted and
  enumerated as `2fe3:0100`, product `AE1200 Strata Demo`.
- Saved the pre-upload firmware locally as
  `build/device-backups/itsybitsy-before-first-flash-20260906.uf2`.
- The user confirmed working display output after the MOSI/MISO wiring
  clarification: display SI (pin 2) connects to MOSI; MISO stays disconnected.
  This is the first working hardware baseline. Exact optical color matching,
  faceplate geometry, and measured frame timing remain uncalibrated.
- Uploaded UF2 SHA-256:
  `ccd3b19e8c17a7e73d04d46ec73dfda870762bf1eae6d670112f0a5301894ac9`.
- Serial status reading is blocked by Linux permissions (`ttyACM0` belongs to
  `dialout`, which the current user is not a member of). Double-press RESET for
  future uploads.

The USB console uses Zephyr's development VID/PID. Assign appropriate USB IDs
before distributing a finished USB product.

## Protocol references

The transport follows the LPM013M126A specification: RGB111 4-bit write command
`0x90`, normal one-based JDI gate addresses, two trailing dummy bytes, active-
high SCS timing, and externally serviced COM inversion. It was cross-checked
against Zephyr's upstream LPM013M126 driver and the hardware-tested
`andelf/memory-lcd-spi` implementation. Packet construction remains in portable
C (`core/src/strata_jdi.c`) so the exact byte stream is host-tested independently
of Zephyr and can move unchanged to nRF54L15.

- JDI LPM013M126A specification mirror:
  <https://manuals.plus/m/ced8f60fbf682c6eefb1be2ad11a2dec45b0741927f600c828850bb85986da2d>
- Zephyr upstream driver:
  <https://github.com/zephyrproject-rtos/zephyr/blob/main/drivers/display/display_lpm013m126.c>
- Tested Rust driver:
  <https://github.com/andelf/memory-lcd-spi>

## Moving to nRF54L15

Keep `core/` and `firmware/src/` unchanged. Add an nRF54L15 board overlay that
provides the same `strata_panel` node and assigns its SPI, SCS, DISP, and
EXTCOMIN pins. This is the only intended board-specific boundary.
