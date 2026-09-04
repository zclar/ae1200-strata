import json
import pathlib
import math

root = pathlib.Path(__file__).resolve().parents[1]
geometry = json.loads((root / "config/faceplates/ae1200-community-r1.json").read_text())
assert geometry["display"]["pixels"] == [176, 176]
assert geometry["display"]["pixel_pitch_mm"] == 0.1308
assert set(geometry["apertures"]) == {"analog", "status", "map", "main"}
assert geometry["cover"]["bounds_xz_mm"] == [[0.0, 12.94375], [30.6276, 42.29375]]
assert len(geometry["decorations"]["labels"]) == 8
assert geometry["decorations"]["analog_ring"]["tick_count"] == 60
assert len(geometry["decorations"]["screws_xz_mm"]) == 4

def bounds(points):
    xs, ys = zip(*points)
    return max(xs) - min(xs), max(ys) - min(ys)

def assert_bounds(name, expected):
    actual = bounds(geometry["apertures"][name]["contour_xz_mm"])
    assert all(math.isclose(a, e, abs_tol=1e-4) for a, e in zip(actual, expected)), (name, actual)

assert_bounds("analog", (8.97483, 9.0))
assert_bounds("status", (10.68, 2.58))
assert_bounds("map", (10.68, 5.96))
assert_bounds("main", (23.28, 9.8))
print("validated four rounded/filleted faceplate contours")
