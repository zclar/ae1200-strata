#!/usr/bin/env python3
import ctypes
import hashlib
import pathlib

root = pathlib.Path(__file__).resolve().parents[1]
lib = ctypes.CDLL(str(root / "build" / "libstrata_display.so"))
lib.strata_scene_count.restype = ctypes.c_uint

size = 176 * 176
frame_type = ctypes.c_uint8 * size
assert lib.strata_scene_count() == 6

hashes = set()
for scene in range(lib.strata_scene_count()):
    first, second = frame_type(), frame_type()
    lib.strata_render(first, scene, 1234)
    lib.strata_render(second, scene, 1234)
    assert bytes(first) == bytes(second), f"scene {scene} is not deterministic"
    assert max(first) <= 7, f"scene {scene} emitted a non-RGB111 color"
    assert len(set(first)) > 1, f"scene {scene} is blank"
    hashes.add(hashlib.sha256(bytes(first)).hexdigest())

assert len(hashes) == lib.strata_scene_count(), "scene outputs are not unique"
print(f"validated {len(hashes)} deterministic RGB111 scenes")
