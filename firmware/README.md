# Hardware firmware

This Zephyr application renders the same portable C framebuffer used by the
desktop emulator and writes it to the JDI LPM013M126A in native RGB111 4-bit
transfer mode. The in-tree driver deliberately owns SCS because this panel's
chip select is active high and has explicit setup/hold timing.

## Build for the Adafruit ItsyBitsy nRF52840 Express

Install Nordic nRF Connect SDK/Zephyr and enter its configured terminal, then
from the repository root run:

```sh
west build -p always -b adafruit_itsybitsy/nrf52840 firmware
```

The current machine does not have `west` or nRF Connect SDK installed, so the
firmware cannot be cross-compiled here yet. The portable renderer and complete
JDI packet format are covered by host tests with `make test`.

To flash through the stock UF2 bootloader:

1. Double-tap reset on the ItsyBitsy; the `ITSY840BOOT` drive appears.
2. Copy `build/zephyr/zephyr.uf2` to that drive.
3. The board restarts and refreshes the animated classic face twice per second;
   EXTCOMIN continues independently at 1 Hz while the panel remains enabled.

The exact FPC wiring and decoupling are in `docs/hardware-wiring.md`. Do not
connect power until adapter pin order has been checked for reversal with a
multimeter.

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
