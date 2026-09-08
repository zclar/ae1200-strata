# Glucose display demo

This is a visual prototype with fictional snapshots. It does not connect to a
CGM, measure glucose, provide treatment advice, or implement medical alarms.
Every card is marked `GLUCOSE DEMO` and `SIMULATED` on the display.

The main segment shows glucose in mg/dL, trend arrow and text, a fictional
24-hour time-in-range percentage (`24H TIR`), and reading age. Five cards use a
categorical circle gauge with a needle and LOW/OK/HIGH text. An additional card
uses the entire circle as a liquid-style level: colored fill reaches the exact
aperture edges, rises from bottom to top for two seconds, and falls for two
seconds while the reading, range color, and trend update with it. A second
level card repeats that motion using black fill only. At either red extreme,
an exclamation mark blinks on for 500 ms and off for 500 ms. Readings and status
remain visible throughout the blink.

| Band | Demo color | Range (mg/dL) | Example |
| --- | --- | --- | --- |
| Very low | Red, blinking exclamation | Below 54 | 48, falling |
| Low | Blue | 54–69 | 63, falling |
| In range | Green | 70–180 | 112, steady |
| High | Yellow | 181–250 | 212, rising |
| Very high | Red, blinking exclamation | Above 250 | 278, rising |

Thresholds use common adult CGM bands described by
[NIDDK](https://www.niddk.nih.gov/health-information/professionals/diabetes-discoveries-practice/clinical-targets-for-continuous-glucose-monitoring-data)
and the [ADA's standardized CGM metrics](https://doi.org/10.2337/dci23-0036).
The colors and extreme-only blinking follow the requested demo design, not a
clinical alert standard. Blue still means low glucose, not safe or in range.
Individual targets and alert requirements differ; a real integration would
need configurable targets, data freshness/error handling, and clinical review.

The on-screen header is `GLUCOSE`; `TARGET 70-180` replaces prototype labeling
with the target range used by these cards. Documentation retains the explicit
simulation warning because the values are fictional.

Each snapshot lasts four seconds: in-range at 16 s, low at 20 s, very-low at
24 s, high at 28 s, and very-high at 32 s. These are separate fictional examples,
not a person changing glucose levels that quickly. The level animation follows
at 36–40 seconds. The black level follows at 40–44 seconds, and the full main
reel repeats at 44 s. During these cards,
the main renderer temporarily owns the circle;
it clears only that aperture and draws after the existing weather renderer.
The middle and upper-right reels keep their timing. At 44 s the circle returns
to its existing reel without retaining the glucose gauge.

Code and timing live in `core/src/strata_display.c`, shared by native emulator
and Nordic firmware. `python3 tests/preview_glucose.py` generates a contact sheet
and an animated preview using the native palette and faceplate mask.
