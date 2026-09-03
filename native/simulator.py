#!/usr/bin/env python3
import ctypes
import json
import pathlib
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
PITCH = GEOMETRY["display"]["pixel_pitch_mm"]
# Center the community cover's measured aperture field over the JDI active area.
X0 = 3.66381 - (23.28 - 23.0208) / 2
Z0 = 17.41375 - (23.0208 - (38.10376 - 17.41375)) / 2
SIZE, SCALE = 176, 3
OUTER_W = round(30.6276 / PITCH * SCALE)
OUTER_H = round(29.3500 / PITCH * SCALE)
PANEL_OX = round(X0 / PITCH * SCALE)
PANEL_OY = round((Z0 - 12.94375) / PITCH * SCALE)
Frame = ctypes.c_uint8 * (SIZE * SIZE)

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

class Simulator:
    def __init__(self, root):
        self.root, self.scene, self.frame = root, 0, Frame()
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

    def select(self, scene):
        self.scene = scene
        self.title.config(text=LIB.strata_scene_name(self.scene).decode())
        self.description.config(text=LIB.strata_scene_description(self.scene).decode())
        self.draw()

    def draw(self):
        LIB.strata_render(self.frame, self.scene, 0)
        header = f"P6\n{SIZE} {SIZE}\n255\n".encode()
        pixels = b''.join(bytes(COLORS[p]) if not self.mask.get() or visible(i % SIZE, i // SIZE)
                           else bytes(FACEPLATE)
                          for i, p in enumerate(self.frame))
        image = tk.PhotoImage(data=header + pixels, format="PPM").zoom(SCALE)
        self.image = image; self.canvas.delete("all")
        self.canvas.create_rectangle(12, 12, OUTER_W - 12, OUTER_H - 12,
                                     fill="#181c18", outline="#697369", width=3)
        self.canvas.create_image(PANEL_OX, PANEL_OY, image=image, anchor="nw")
        if self.mask.get():
            for polygon in APERTURES:
                points = [(PANEL_OX + x * SCALE, PANEL_OY + y * SCALE) for x, y in polygon]
                self.canvas.create_polygon(points, outline="#697369", fill="", width=1)

if __name__ == "__main__":
    window = tk.Tk(); Simulator(window); window.mainloop()
