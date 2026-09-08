#!/usr/bin/env python3
"""Preview all five glucose demo cards through the native faceplate mask."""
from PIL import Image, ImageDraw

from preview_weather import ROOT, render


if __name__ == "__main__":
    times = (16000, 20000, 24000, 28000, 32000, 37000, 38000, 39000,
             41000, 42000, 43000)
    labels = ("IN RANGE", "LOW", "VERY LOW", "HIGH", "VERY HIGH",
              "COLOR LEVEL RISING", "COLOR LEVEL PEAK", "COLOR LEVEL FALLING",
              "BLACK LEVEL RISING", "BLACK LEVEL PEAK", "BLACK LEVEL FALLING")
    sheet = Image.new("RGB", (528 * 4, 552 * 3), "#181c18")
    draw = ImageDraw.Draw(sheet)
    for i, ms in enumerate(times):
        x, y = i % 4 * 528, i // 4 * 552
        sheet.paste(render(ms).resize((528, 528), Image.Resampling.NEAREST), (x, y + 24))
        draw.text((x + 8, y + 7), labels[i], fill="white")
    sheet.save(ROOT / "build/glucose-preview.png")
    frames = [render(ms).resize((528, 528), Image.Resampling.NEAREST)
              for ms in range(16000, 44000, 100)]
    frames[0].save(ROOT / "build/glucose-demo.gif", save_all=True,
                   append_images=frames[1:], duration=100, loop=0, disposal=2)
    print("Wrote build/glucose-preview.png and build/glucose-demo.gif")
