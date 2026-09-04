#!/usr/bin/env python3
import ctypes
import hashlib
import pathlib

root = pathlib.Path(__file__).resolve().parents[1]
lib = ctypes.CDLL(str(root / "build" / "libstrata_display.so"))
lib.strata_scene_count.restype = ctypes.c_uint

size = 176 * 176
frame_type = ctypes.c_uint8 * size
packed_type = ctypes.c_uint8 * (176 // 2)
packet_type = ctypes.c_uint8 * 92
clear_type = ctypes.c_uint8 * 2
assert lib.strata_scene_count() == 1

hashes = set()
for scene in range(lib.strata_scene_count()):
    first, second = frame_type(), frame_type()
    lib.strata_render(first, scene, 1234)
    lib.strata_render(second, scene, 1234)
    assert bytes(first) == bytes(second), f"scene {scene} is not deterministic"
    assert max(first) <= 7, f"scene {scene} emitted a non-RGB111 color"
    assert len(set(first)) > 1, f"scene {scene} is blank"
    assert 1 in first, f"scene {scene} is missing its blue Bluetooth/time-zone accents"
    hashes.add(hashlib.sha256(bytes(first)).hexdigest())

assert len(hashes) == lib.strata_scene_count(), "scene outputs are not unique"

# Hardware stream packs two RGB111 pixels into RGB0/RGB0 nibbles.
pattern, packed = frame_type(), packed_type()
for x in range(176):
    pattern[x] = x % 8
lib.strata_pack_line_rgb111(pattern, 0, packed)
assert list(packed[:4]) == [0x02, 0x46, 0x8A, 0xCE]

# A hardware line packet is command + normal 1-based JDI row + RGB0 data
# + 16 dummy clocks. Bit reversal is used by Sharp panels, not this JDI panel.
packet = packet_type()
assert lib.strata_jdi_encode_line(pattern, 0, packet, len(packet)) == 92
assert packet[0] == 0x90 and packet[1] == 0x01
assert list(packet[2:6]) == [0x02, 0x46, 0x8A, 0xCE]
assert list(packet[-2:]) == [0, 0]
assert lib.strata_jdi_encode_line(pattern, 175, packet, len(packet)) == 92
assert packet[1] == 176
assert lib.strata_jdi_encode_line(pattern, 176, packet, len(packet)) == -1

clear = clear_type()
assert lib.strata_jdi_encode_all_clear(clear, len(clear)) == 2
assert list(clear) == [0x20, 0]
print(f"validated {len(hashes)} deterministic RGB111 scenes")
