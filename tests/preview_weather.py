#!/usr/bin/env python3
"""Optional visual review: shared renderer, native palette and aperture mask.

Run after make test; requires Pillow. Outputs are ignored build artifacts.
"""
import importlib.util
from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("native_preview", ROOT / "native/simulator.py")
native = importlib.util.module_from_spec(spec)
spec.loader.exec_module(native)


def render(ms):
    frame = native.Frame()
    native.LIB.strata_render(frame, 0, ms)
    result = Image.new("RGB", (176, 176))
    result.putdata([native.COLORS[p] if native.VISIBLE_PIXELS[i] else native.FACEPLATE
                    for i, p in enumerate(frame)])
    return result


def circle(ms):
    return render(ms).crop((2, 14, 74, 84)).resize((432, 420), Image.Resampling.NEAREST)


if __name__ == "__main__":
    times = (2000, 2600, 3200, 3800, 6200, 10400)
    sheet = Image.new("RGB", (432 * 3, 448 * 2), "#181c18")
    draw = ImageDraw.Draw(sheet)
    for i, ms in enumerate(times):
        x, y = i % 3 * 432, i // 3 * 448
        sheet.paste(circle(ms), (x, y + 28))
        draw.text((x + 8, y + 8), f"{ms} ms / native palette and clipping", fill="white")
    sheet.save(ROOT / "build/weather-preview.png")
    frames = []
    for elapsed in range(0, 2400, 100):
        pair = Image.new("RGB", (864, 420))
        pair.paste(circle(2000 + elapsed), (0, 0))
        pair.paste(circle(6000 + elapsed), (432, 0))
        frames.append(pair)
    frames[0].save(ROOT / "build/weather-animation.gif", save_all=True,
                   append_images=frames[1:], duration=100, loop=0, disposal=2)
    render(6500).resize((704, 704), Image.Resampling.NEAREST).save(
        ROOT / "build/weather-faceplate.png")
    print("Wrote build/weather-preview.png, weather-animation.gif, weather-faceplate.png")
