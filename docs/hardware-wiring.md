# nRF52840 development wiring

This is the reserved bring-up map for the Adafruit ItsyBitsy nRF52840 Express
and the JDI LPM013M126A FPC. Pin numbers below are **JDI FPC pin numbers**, not
the numbering printed on an FPC-to-DIP adapter. Continuity-check the adapter:
bottom-contact connectors can mirror the apparent pin order.

| JDI pin | Signal | ItsyBitsy connection |
|---:|---|---|
| 1 | SCLK | hardware SPI SCK, P0.13 |
| 2 | SI | hardware SPI MOSI, P0.15 |
| 3 | SCS | D10 / P0.05, active-high chip select |
| 4 | EXTCOMIN | D7 / P1.08, periodic GPIO/PWM |
| 5 | DISP | D9 / P0.07, display-enable GPIO |
| 6 | VDDA | 3V rail |
| 7 | VDD | 3V rail |
| 8 | EXTMODE | 3V/VDD for external COM inversion |
| 9 | VSS | GND |
| 10 | VSSA | GND |

MISO is unused. Start SPI at 1 MHz, mode 0, MSB first; the panel specification
limit is 2 MHz. Keep the panel on the regulated 3V/3.3V rail only—never VHI,
USB, or BAT. Add local decoupling at the FPC: 100 nF from VDDA to VSSA and
1 µF from VDD to VSS. The datasheet reference also shows 100 nF on DISP, but
that capacitor can slow a GPIO-driven DISP edge; leave it unstuffed for first
bring-up and verify the edge on a scope.

With EXTMODE high, EXTCOMIN must continue toggling while the panel is powered
and enabled. Use a stable square wave; 1 Hz is valid for initial reflective
bring-up, while a higher rate can be selected after checking optical flicker.

The shared C renderer emits the framebuffer and packs each line as two
RGB111 pixels in `RGB0/RGB0` nibbles. The transport driver should remain a
separate Zephyr layer so this wiring can move to nRF54L15 Devicetree pins
without changing scene/layout code.
