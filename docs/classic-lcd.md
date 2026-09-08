# Classic LCD digit reference

The clock's electrode maps in `core/src/strata_display.c` are adapted from
the rightmost large **8** in the straight-on LCD close-up on
[Casio's AE-1200 details page](https://www.casio.com/us/watches/casio/about/ae-1200/).
That digit exposes all seven electrodes. This is a visual approximation of the
photograph, not Casio's original electrode artwork or font file.

Reference image: `feature-2-2-en.jpeg`, 1200 x 800, showing 9:08:36.
The traced area is x=657..738, y=514..649. The hand-selected electrode
vertices below are in photograph coordinates, before conversion to MIP pixels:

| Electrode | Outline vertices (x, y) |
| --- | --- |
| A, top | (683,515), (729,515), (710,532), (695,532) |
| B, upper right | (733,517), (737,529), (734,551), (729,573), (711,566), (715,536) |
| C, lower right | (728,583), (725,613), (714,644), (703,627), (709,593) |
| D, bottom | (660,639), (678,630), (698,630), (707,647), (661,647), (658,643) |
| E, lower left | (660,634), (664,602), (668,584), (683,592), (678,625) |
| F, upper left | (679,520), (691,536), (686,566), (666,575), (670,547) |
| G, middle | (665,579), (685,570), (707,570), (724,579), (706,588), (684,588) |

The shapes are reduced to 24 x 41 cells for hours/minutes and 17 x 29 for
seconds, using 8 x 8 coverage samples per cell and a 30/64 coverage threshold.
Where different electrodes would share a cell edge, the lower-coverage cell
is left blank (the later cell on ties). This keeps the diagonal LCD gaps visible
on a binary pixel grid. A..G in the checked-in maps identify electrodes; dots
are unlit cells. Rendering performs only a table lookup and a segment-mask
check, with no runtime scaling, antialiasing, or polygon rasterization.

Both halves of each digit lean the same way. The seconds share the large
digits' baseline and occupy roughly 70% of their height, as in the reference.
Normal and inverted cards use identical maps in the shared C renderer compiled
by both the desktop emulator and Nordic firmware.

The stock LCD has continuously shaped electrodes. A 176 x 176 MIP matrix
cannot reproduce every curve or its subpixel-width gaps exactly. The small
day/date and status legends still use the existing pixel lettering; they have
not been traced from the stock alphanumeric LCD.

Run `make test` and `python3 tests/preview_classic.py` to generate the normal
and inverted comparison in `build/classic-inverted-preview.png`.
