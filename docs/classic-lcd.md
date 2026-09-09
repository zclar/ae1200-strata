# Classic LCD digits

The clock uses conventional seven-segment LCD geometry in
`core/src/strata_display.c`: straight stems, matching 45-degree ends, a
centered middle bar, and mirrored upper/lower and left/right electrodes.
The main stems are five pixels thick; seconds stems are four pixels thick.
Hours/minutes occupy 24 x 41 cells and seconds 17 x 29, with a common baseline.
The colon dots are aligned vertically.

This replaces the earlier photograph-derived outlines with regular LCD
segments. The stock layout was referenced from
[Casio's AE-1200 details page](https://www.casio.com/us/watches/casio/about/ae-1200/),
but these are standard LCD digits, not Casio's original electrode artwork.

A..G in the checked-in maps identify electrodes; dots are unlit gaps.
Rendering performs only a table lookup and a segment-mask check. There is
no runtime scaling, antialiasing, or polygon rasterization. The outlines stay
on the native 176 x 176 RGB111 pixel grid, including their diagonal ends.

Normal and inverted cards use identical maps in the shared C renderer
compiled by both the desktop emulator and Nordic firmware. The small
day/date and status legends retain the existing pixel lettering.

Run `make test` and `python3 tests/preview_classic.py` to generate the normal
and inverted comparison in `build/classic-inverted-preview.png`.
