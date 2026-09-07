#!/usr/bin/env python3
"""Render the music card using the shared C library and native faceplate mask."""
from PIL import Image

from preview_weather import ROOT, render


if __name__ == "__main__":
    frames = [render(12000 + ms).resize((704, 704), Image.Resampling.NEAREST)
              for ms in range(0, 4000, 50)]
    frames[0].save(ROOT / "build/music-preview.png")
    frames[0].save(ROOT / "build/music-playback.gif", save_all=True,
                   append_images=frames[1:], duration=50, loop=0, disposal=2)
    print("Wrote build/music-preview.png and build/music-playback.gif")
