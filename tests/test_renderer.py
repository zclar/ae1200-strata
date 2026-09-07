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

animated = frame_type()
lib.strata_render(animated, 0, 2234)
assert bytes(animated) != bytes(first), "classic scene did not advance with elapsed time"

battery = frame_type()
lib.strata_render(battery, 0, 2000)
assert 2 in battery, "battery reel item is missing its RGB111 green level fill"
classic_status_frame = frame_type()
lib.strata_render(classic_status_frame, 0, 999)
assert any(battery[y * 176 + x] != classic_status_frame[y * 176 + x]
           for y in range(7, 29) for x in range(95, 176)), \
    "upper status reel did not advance"

notification = frame_type()
lib.strata_render(notification, 0, 6000)
assert notification[122 * 176 + 10] == 1, "message reel is missing its blue envelope"
assert notification[99 * 176 + 96] == 0, "message reel is missing its notification header"
assert bytes(notification) != bytes(first), "main display reel did not advance"

gmail = frame_type()
lib.strata_render(gmail, 0, 10000)
assert gmail[123 * 176 + 8] == 4, "Gmail reel is missing its red M pillar"
assert gmail[99 * 176 + 137] == 4, "Gmail reel is missing its red header"
assert bytes(gmail) != bytes(notification), "Gmail reel did not advance"

# Every reel item owns exactly [0, 4000) ms of the shared timeline.
before_messages, start_messages = frame_type(), frame_type()
before_gmail, start_gmail = frame_type(), frame_type()
before_repeat, repeat = frame_type(), frame_type()
lib.strata_render(before_messages, 0, 3999)
lib.strata_render(start_messages, 0, 4000)
lib.strata_render(before_gmail, 0, 7999)
lib.strata_render(start_gmail, 0, 8000)
lib.strata_render(before_repeat, 0, 11999)
lib.strata_render(repeat, 0, 12000)
assert before_messages[122 * 176 + 10] != 1
assert start_messages[122 * 176 + 10] == 1
assert before_gmail[123 * 176 + 8] != 4
assert start_gmail[123 * 176 + 8] == 4
assert before_repeat[123 * 176 + 8] == 4
assert repeat[123 * 176 + 8] != 4

def render_at(ms):
    frame = frame_type()
    lib.strata_render(frame, 0, ms)
    return bytes(frame)

def region(frame, x, y, width, height):
    return b''.join(frame[row * 176 + x:row * 176 + x + width]
                    for row in range(y, y + height))

status_box = (95, 7, 81, 22)
circle_box = (3, 15, 70, 68)
middle_box = (98, 39, 74, 42)
main_box = (0, 95, 176, 70)

# Phase offsets stagger slot changes: status at 1s, paired weather at 2s,
# main at 4s, then each group changes items every four seconds.
assert 2 not in region(render_at(999), *status_box)
assert 2 in region(render_at(1000), *status_box)
assert 2 in region(render_at(4999), *status_box)
assert 2 in region(render_at(5000), *status_box)
assert 2 in region(render_at(8999), *status_box)
assert 2 not in region(render_at(9000), *status_box)
assert region(render_at(1000), *status_box) != region(render_at(5000), *status_box), \
    "plain and percentage battery items must remain distinct"
assert 6 not in region(render_at(1999), *circle_box)
assert 6 in region(render_at(2000), *circle_box)
assert 6 in region(render_at(5999), *circle_box)
assert 6 not in region(render_at(6000), *circle_box)
rain_box = (3, 68, 70, 14)
assert 1 not in region(render_at(9999), *rain_box)
assert 1 in region(render_at(10000), *rain_box)
assert 1 in region(render_at(13999), *rain_box)
assert 6 in region(render_at(14000), *circle_box), "storm lightning is missing"
assert 6 not in region(render_at(14800), *circle_box), "lightning did not blink off"
assert 1 not in region(render_at(18000), *circle_box)

for ms in (2000, 6000, 10000, 14000, 18000):
    before, after = render_at(ms - 1), render_at(ms)
    assert region(before, *middle_box) != region(after, *middle_box), "weather data missed icon transition"
    assert region(before, *status_box) == region(after, *status_box), "status changed with weather"

# Weather only owns the circle + middle: main slot transitions don't redraw it.
assert region(render_at(3999), *middle_box) == region(render_at(4000), *middle_box)
assert region(render_at(11999), *middle_box) == region(render_at(12000), *middle_box)
for ms in (2100, 6100, 10100, 14100):
    initial, later = render_at(ms), render_at(ms + 300)
    assert region(initial, *circle_box) != region(later, *circle_box), "weather icon is static"
    assert region(initial, *middle_box) == region(later, *middle_box), "weather readings drifted"
    assert max(initial) <= 7
detail_box = (101, 73, 48, 7)
assert 1 in region(render_at(10000), *detail_box), "rain probability must be blue"
assert 1 not in region(render_at(6000), *detail_box), "cloudy label should not imply rain"
assert region(render_at(2000), *middle_box) == region(render_at(22000), *middle_box)
for ms in (2000, 6000, 10000):
    assert set(region(render_at(ms), 146, 42, 23, 7)) == {7}, "DEMO label was not removed"
for ms in (6000, 10000, 14800):
    cloud = region(render_at(ms), 9, 31, 60, 32)
    assert set(cloud) == {0, 7}, "cloud body/shadows must use real black/white pixels"

# Large timestamps must still select valid RGB111 frames.
assert max(render_at(0xffffffff)) <= 7

# Hardware stream packs two RGB111 pixels into RGB0/RGB0 nibbles.
pattern, packed = frame_type(), packed_type()
for x in range(176):
    pattern[x] = x % 8
lib.strata_pack_line_rgb111(pattern, 0, packed)
assert list(packed[:4]) == [0x02, 0x46, 0x8A, 0xCE]

# A background pixel is hardware white, not the desktop's optical gray tint.
assert first[0] == 7, "demo background must use RGB111 white"
for color, expected in ((0, 0x00), (1, 0x22), (2, 0x44), (4, 0x88), (7, 0xEE)):
    for x in range(176):
        pattern[x] = color
    lib.strata_pack_line_rgb111(pattern, 0, packed)
    assert all(byte == expected for byte in packed), f"incorrect color encoding: {color}"
for x in range(176):
    pattern[x] = x % 8

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
