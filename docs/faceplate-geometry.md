# AE-1200 faceplate geometry

The repository now contains a machine-readable reference at
`config/faceplates/ae1200-community-r1.json`. It is derived from the community
STL [Casio Royale (AE1200) Faceplate - Full Detail](https://www.thingiverse.com/thing:6825880),
not an official Casio drawing. The STL is licensed CC BY-SA and was reported by
its creator/community testers to fit a printed replacement.

The extracted front-plane contours preserve the rounded/filleted corners and
the sculpted upper-left boundary of the main opening. There are four openings:

- analog: clear Ø9.00 mm; the Ø12.75 mm figure is the outer decorative surround;
- upper-right status: 10.68 × 2.58 mm, rounded corners;
- upper-right map: 10.68 × 5.96 mm, rounded corners;
- lower main opening: 23.28 × 9.80 mm bounding box, sculpted upper-left edge.

The native emulator also renders the cover's minute track, minute numerals,
four screw heads, and the printed `5 ALARMS`, `CASIO`, `ADJUST`, `LIGHT`, `MODE`,
`SEARCH`, `WR100M`, and `10 YEAR BATTERY` legends. Their visual placement follows
[Casio's official AE-1200WH-1AV product photography](https://www.casio.com/intl/watches/casio/product.AE-1200WH-1AV/),
but they are reference artwork rather than manufacturing dimensions. The four
aperture contours remain the geometry used for pixel clipping.

The JDI LPM013M126A active area is 23.0208 × 23.0208 mm (176 × 176 pixels,
0.1308 mm pitch). Therefore the main cover opening is 0.2592 mm wider than
the active area. The reference alignment centers the active matrix, leaving
0.1296 mm (approximately one pixel pitch) of inactive LCD glass visible at each
side of the main opening. The emulator renders that glass in LCD gray but never
places addressable graphics there. This does not prove a fit problem—the module
itself is wider than the opening—but the origin still needs physical validation.

Before freezing a mask, provide a straight-on CAD view or a calibrated photo
of the actual cover and the panel. Record the active-area origin, rotation,
and scale; then replace the `alignment` block and use the contours directly.
