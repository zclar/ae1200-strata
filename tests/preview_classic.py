#!/usr/bin/env python3
"""Preview the refined normal and inverted AE-1200 time displays."""
from PIL import Image, ImageDraw

from preview_weather import ROOT, render


if __name__ == "__main__":
    frames = (render(0), render(44000))
    sheet = Image.new("RGB", (704 * 2, 736), "#181c18")
    draw = ImageDraw.Draw(sheet)
    for i, (label, frame) in enumerate(zip(("CLASSIC LCD", "INVERTED LCD"), frames)):
        sheet.paste(frame.resize((704, 704), Image.Resampling.NEAREST), (i * 704, 32))
        draw.text((i * 704 + 12, 10), label, fill="white")
    sheet.save(ROOT / "build/classic-inverted-preview.png")
    print("Wrote build/classic-inverted-preview.png")
