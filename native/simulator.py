#!/usr/bin/env python3
import ctypes
import json
import math
import pathlib
import time
import tkinter as tk

ROOT = pathlib.Path(__file__).resolve().parents[1]
LIB = ctypes.CDLL(str(ROOT / "build" / "libstrata_display.so"))
GEOMETRY = json.loads((ROOT / "config" / "faceplates" / "ae1200-community-r1.json").read_text())
LIB.strata_scene_count.restype = ctypes.c_uint
LIB.strata_scene_name.restype = ctypes.c_char_p
LIB.strata_scene_description.restype = ctypes.c_char_p

COLORS = ((24, 29, 24), (61, 102, 142), (78, 140, 73), (101, 163, 157),
          (184, 59, 50), (140, 86, 125), (201, 184, 62), (169, 176, 162))
FACEPLATE = (24, 28, 24)
LCD_BACKGROUND = "#a9b0a2"
PITCH = GEOMETRY["display"]["pixel_pitch_mm"]
COVER_MIN, COVER_MAX = GEOMETRY["cover"]["bounds_xz_mm"]
X0, Z0 = GEOMETRY["alignment"]["active_origin_xz_mm"]
SIZE, SCALE = 176, 3
OUTER_W = round((COVER_MAX[0] - COVER_MIN[0]) / PITCH * SCALE)
OUTER_H = round((COVER_MAX[1] - COVER_MIN[1]) / PITCH * SCALE)
PANEL_OX = round((X0 - COVER_MIN[0]) / PITCH * SCALE)
PANEL_OY = round((Z0 - COVER_MIN[1]) / PITCH * SCALE)
Frame = ctypes.c_uint8 * (SIZE * SIZE)
LIB.strata_render.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.c_uint,
                              ctypes.c_uint32]
LIB.strata_render.restype = None

def inside(point, polygon):
    x, y = point; hit = False
    for i, (ax, ay) in enumerate(polygon):
        bx, by = polygon[i - 1]
        if (ay > y) != (by > y) and x < (bx - ax) * (y - ay) / (by - ay) + ax:
            hit = not hit
    return hit

APERTURES = []
for aperture in GEOMETRY["apertures"].values():
    APERTURES.append(tuple(((x - X0) / PITCH, (z - Z0) / PITCH)
                           for x, z in aperture["contour_xz_mm"]))

def visible(x, y):
    return any(inside((x + .5, y + .5), polygon) for polygon in APERTURES)

def faceplate_point(x, z):
    return ((x - COVER_MIN[0]) / PITCH * SCALE,
            (z - COVER_MIN[1]) / PITCH * SCALE)

class Simulator:
    def __init__(self, root):
        self.root, self.scene, self.frame = root, 0, Frame()
        self.started = time.monotonic()
        root.title("AE1200 Emulator — Native Simulator")
        root.configure(bg="#111410")
        self.canvas = tk.Canvas(root, width=OUTER_W, height=OUTER_H,
                                bg="#111610", highlightthickness=12,
                                highlightbackground="#30362f")
        self.canvas.grid(row=0, column=0, rowspan=4, padx=24, pady=24)
        self.title = tk.Label(root, fg="#e9eee5", bg="#111410", font=("Sans", 18, "bold"))
        self.title.grid(row=0, column=1, sticky="sw", padx=(0, 24))
        self.description = tk.Label(root, fg="#aeb6aa", bg="#111410", wraplength=230, justify="left")
        self.description.grid(row=1, column=1, sticky="nw", padx=(0, 24))
        self.mask = tk.BooleanVar(value=True)
        tk.Checkbutton(root, text="Reference faceplate mask (unverified)", variable=self.mask, command=self.draw,
                       fg="#c2c9bd", bg="#111410", selectcolor="#242a22").grid(row=2, column=1, sticky="nw")
        tk.Label(root, text="30.63 × 29.35 mm cover  •  23.02 mm active", fg="#788174", bg="#111410").grid(row=3, column=1, sticky="nw")
        self.select(0)
        self.root.after(250, self.animate)

    def animate(self):
        self.draw()
        self.root.after(250, self.animate)

    def select(self, scene):
        self.scene = scene
        self.title.config(text=LIB.strata_scene_name(self.scene).decode())
        self.description.config(text=LIB.strata_scene_description(self.scene).decode())
        self.draw()

    def draw(self):
        elapsed_ms = int((time.monotonic() - self.started) * 1000) & 0xffffffff
        LIB.strata_render(self.frame, self.scene, elapsed_ms)
        header = f"P6\n{SIZE} {SIZE}\n255\n".encode()
        pixels = b''.join(bytes(COLORS[p]) if not self.mask.get() or visible(i % SIZE, i // SIZE)
                           else bytes(FACEPLATE)
                          for i, p in enumerate(self.frame))
        image = tk.PhotoImage(data=header + pixels, format="PPM").zoom(SCALE)
        self.image = image; self.canvas.delete("all")
        self.canvas.create_rectangle(12, 12, OUTER_W - 12, OUTER_H - 12,
                                     fill="#181c18", outline="#697369", width=3)
        if self.mask.get():
            # The module glass is wider than its active matrix. Keep the narrow
            # inactive margin LCD-gray instead of exposing black cover beneath it.
            for aperture in GEOMETRY["apertures"].values():
                points = [faceplate_point(x, z) for x, z in aperture["contour_xz_mm"]]
                self.canvas.create_polygon(points, fill=LCD_BACKGROUND, outline="")
        self.canvas.create_image(PANEL_OX, PANEL_OY, image=image, anchor="nw")
        if self.mask.get():
            for polygon in APERTURES:
                points = [(PANEL_OX + x * SCALE, PANEL_OY + y * SCALE) for x, y in polygon]
                self.canvas.create_polygon(points, outline="#697369", fill="", width=1)
            self.draw_faceplate_details()

    def draw_faceplate_details(self):
        details = GEOMETRY["decorations"]
        colors = {"silver": "#d4dbd0", "red": "#b83d42"}

        # The minute track is printed on the cover around the circular aperture.
        ring = details["analog_ring"]
        cx, cy = faceplate_point(*ring["center_xz_mm"])
        for radius, color, width in ((ring["aperture_radius_mm"], "#747e73", 1),
                                     (ring["outer_radius_mm"], "#3e463f", 2)):
            radius_px = radius / PITCH * SCALE
            self.canvas.create_oval(cx - radius_px, cy - radius_px,
                                    cx + radius_px, cy + radius_px,
                                    outline=color, width=width)
        for tick in range(ring["tick_count"]):
            angle = math.radians(tick * 6 - 90)
            inner = ring["tick_inner_radius_mm"]
            outer = ring["tick_outer_radius_mm"] + (0.22 if tick % 5 == 0 else 0)
            x1, y1 = cx + math.cos(angle) * inner / PITCH * SCALE, cy + math.sin(angle) * inner / PITCH * SCALE
            x2, y2 = cx + math.cos(angle) * outer / PITCH * SCALE, cy + math.sin(angle) * outer / PITCH * SCALE
            self.canvas.create_line(x1, y1, x2, y2, fill="#d4dbd0", width=2 if tick % 5 == 0 else 1)
        for tick in range(0, 60, 5):
            angle = math.radians(tick * 6 - 90)
            radius = ring["number_radius_mm"] / PITCH * SCALE
            value = "60" if tick == 0 else f"{tick:02d}"
            self.canvas.create_text(cx + math.cos(angle) * radius,
                                    cy + math.sin(angle) * radius,
                                    text=value, fill="#d4dbd0",
                                    font=("DejaVu Sans", -10, "bold"))

        for x, z in details["screws_xz_mm"]:
            sx, sy = faceplate_point(x, z)
            radius = 0.39 / PITCH * SCALE
            self.canvas.create_oval(sx - radius, sy - radius, sx + radius, sy + radius,
                                    fill="#111511", outline="#626b62", width=2)
            self.canvas.create_line(sx - radius * .45, sy, sx + radius * .45, sy,
                                    fill="#778077", width=1)

        for item in details["labels"]:
            x, y = faceplate_point(*item["center_xz_mm"])
            pixel_height = max(8, round(item["size_mm"] / PITCH * SCALE))
            self.canvas.create_text(x, y, text=item["text"],
                                    fill=colors[item["color"]],
                                    angle=item.get("angle_deg", 0),
                                    font=("DejaVu Sans", -pixel_height, "bold"))

if __name__ == "__main__":
    window = tk.Tk(); Simulator(window); window.mainloop()
