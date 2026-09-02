#!/usr/bin/env python3
import ctypes
import pathlib
import time
import tkinter as tk

ROOT = pathlib.Path(__file__).resolve().parents[1]
LIB = ctypes.CDLL(str(ROOT / "build" / "libstrata_display.so"))
LIB.strata_scene_count.restype = ctypes.c_uint
LIB.strata_scene_name.restype = ctypes.c_char_p
LIB.strata_scene_description.restype = ctypes.c_char_p

SIZE, SCALE, SCENE_MS = 176, 3, 5000
Frame = ctypes.c_uint8 * (SIZE * SIZE)
COLORS = ((17, 22, 16), (61, 102, 142), (78, 140, 73), (101, 163, 157),
          (184, 59, 50), (140, 86, 125), (201, 184, 62), (220, 224, 200))

class Simulator:
    def __init__(self, root):
        self.root, self.scene, self.playing = root, 0, True
        self.started, self.frame = time.monotonic(), Frame()
        root.title("AE1200 Strata — Native Simulator")
        root.configure(bg="#111410")
        self.canvas = tk.Canvas(root, width=SIZE*SCALE, height=SIZE*SCALE,
                                bg="#111610", highlightthickness=12,
                                highlightbackground="#30362f")
        self.canvas.grid(row=0, column=0, rowspan=5, padx=24, pady=24)
        self.title = tk.Label(root, fg="#e9eee5", bg="#111410", font=("Sans", 18, "bold"))
        self.title.grid(row=0, column=1, sticky="sw", padx=(0, 24))
        self.description = tk.Label(root, fg="#aeb6aa", bg="#111410", wraplength=230, justify="left")
        self.description.grid(row=1, column=1, sticky="nw", padx=(0, 24))
        controls = tk.Frame(root, bg="#111410"); controls.grid(row=2, column=1, sticky="new", padx=(0, 24))
        tk.Button(controls, text="←", command=lambda: self.select(-1)).pack(side="left")
        self.play = tk.Button(controls, text="Pause", command=self.toggle); self.play.pack(side="left", padx=8)
        tk.Button(controls, text="→", command=lambda: self.select(1)).pack(side="left")
        self.mask = tk.BooleanVar(value=True)
        tk.Checkbutton(root, text="Faceplate mask", variable=self.mask, command=self.draw,
                       fg="#c2c9bd", bg="#111410", selectcolor="#242a22").grid(row=3, column=1, sticky="nw")
        tk.Label(root, text="176 × 176  •  RGB111  •  shared C renderer", fg="#788174", bg="#111410").grid(row=4, column=1, sticky="nw")
        self.select(0, absolute=True); self.tick()

    def select(self, amount, absolute=False):
        self.scene = amount % LIB.strata_scene_count() if absolute else (self.scene + amount) % LIB.strata_scene_count()
        self.started = time.monotonic()
        self.title.config(text=LIB.strata_scene_name(self.scene).decode())
        self.description.config(text=LIB.strata_scene_description(self.scene).decode())
        self.draw()

    def toggle(self):
        self.playing = not self.playing; self.started = time.monotonic()
        self.play.config(text="Pause" if self.playing else "Play")

    def draw(self):
        elapsed = int((time.monotonic() - self.started) * 1000)
        LIB.strata_render(self.frame, self.scene, elapsed)
        header = f"P6\n{SIZE} {SIZE}\n255\n".encode()
        pixels = b''.join(bytes(COLORS[p]) for p in self.frame)
        image = tk.PhotoImage(data=header + pixels, format="PPM").zoom(SCALE)
        self.image = image; self.canvas.delete("all"); self.canvas.create_image(0, 0, image=image, anchor="nw")
        if self.mask.get():
            s = SCALE; fill = "#181c18"; outline = "#303630"
            self.canvas.create_rectangle(0, 0, 6*s, SIZE*s, fill=fill, outline=outline)
            self.canvas.create_rectangle(169*s, 0, SIZE*s, SIZE*s, fill=fill, outline=outline)
            self.canvas.create_rectangle(0, 0, SIZE*s, 7*s, fill=fill, outline=outline)
            self.canvas.create_rectangle(0, 69*s, SIZE*s, 76*s, fill=fill, outline=outline)
            self.canvas.create_rectangle(70*s, 0, 76*s, 70*s, fill=fill, outline=outline)
            self.canvas.create_rectangle(0, 169*s, SIZE*s, SIZE*s, fill=fill, outline=outline)

    def tick(self):
        elapsed = (time.monotonic() - self.started) * 1000
        if self.playing and elapsed >= SCENE_MS: self.select(1)
        elif self.scene == 5: self.draw()
        self.root.after(100, self.tick)

if __name__ == "__main__":
    window = tk.Tk(); Simulator(window); window.mainloop()
