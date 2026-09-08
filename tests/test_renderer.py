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
before_music, music = frame_type(), frame_type()
lib.strata_render(before_messages, 0, 3999)
lib.strata_render(start_messages, 0, 4000)
lib.strata_render(before_gmail, 0, 7999)
lib.strata_render(start_gmail, 0, 8000)
lib.strata_render(before_music, 0, 11999)
lib.strata_render(music, 0, 12000)
lib.strata_render(before_repeat, 0, 43999)
lib.strata_render(repeat, 0, 44000)
assert before_messages[122 * 176 + 10] != 1
assert start_messages[122 * 176 + 10] == 1
assert before_gmail[123 * 176 + 8] != 4
assert start_gmail[123 * 176 + 8] == 4
assert before_music[123 * 176 + 8] == 4
assert music[123 * 176 + 8] != 4
assert music[158 * 176 + 8] == 1
assert before_repeat[99 * 176 + 127] == 0
assert repeat[99 * 176 + 127] == 7

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

# Music advances within its four-second slot without touching the other reels.
assert region(render_at(12000), 8, 120, 160, 14) == region(render_at(12024), 8, 120, 160, 14)
assert region(render_at(12000), 8, 120, 160, 14) != region(render_at(12025), 8, 120, 160, 14)
assert region(render_at(12000), 8, 141, 28, 19) != region(render_at(12300), 8, 141, 28, 19)
assert region(render_at(12000), 64, 153, 104, 5) != region(render_at(15999), 64, 153, 104, 5)
assert region(render_at(15999), *middle_box) == region(render_at(16000), *middle_box)
assert region(render_at(12000), 64, 152, 35, 8) == region(render_at(12200), 64, 152, 35, 8)
assert region(render_at(12000), 106, 152, 60, 8) == region(render_at(12200), 106, 152, 60, 8)

# Glucose owns main+circle for five snapshots, with blinking confined to the
# exclamation mark at red extremes. Numeric readings must never blink away.
for start, color, alert in ((16000, 2, False), (20000, 1, False),
                            (24000, 4, True), (28000, 6, False), (32000, 4, True)):
    on, off = render_at(start), render_at(start + 500)
    assert set(region(on, 8, 120, 54, 21)) == {7, color}
    assert region(on, *main_box) == region(off, *main_box)
    assert (4 in region(on, 36, 36, 4, 10)) == alert
    assert 4 not in region(off, 36, 36, 4, 10)
    assert region(on, *middle_box) == region(render_at(start % 20000), *middle_box)
    assert region(on, *main_box) == region(render_at(start + 3599), *main_box)
assert region(render_at(35999), *circle_box) != region(render_at(36000), *circle_box)

# The additional full-circle level rises for two seconds, then falls. Its
# reading, color band, direction, and fill height must move together.
level_low, level_green, level_high, level_falling = (
    render_at(36000), render_at(37000), render_at(38000), render_at(39000))
assert 4 in region(level_low, *circle_box)
assert 2 in region(level_green, *circle_box)
assert 4 in region(level_high, *circle_box)
assert 2 in region(level_falling, *circle_box)
assert region(level_low, *circle_box) != region(level_green, *circle_box)
assert region(level_green, *circle_box) != region(level_high, *circle_box)
assert region(level_low, *main_box) != region(level_green, *main_box)
assert region(level_green, *main_box) != region(level_high, *main_box)
assert set(level_high[30 * 176 + 3:30 * 176 + 73]) == {4}, \
    "colored level did not reach both aperture edges"
assert region(level_green, *middle_box) == region(render_at(17000), *middle_box)
assert region(level_high, *middle_box) == region(render_at(18000), *middle_box)
assert region(render_at(39999), *circle_box) != region(render_at(40000), *circle_box)
mono_low, mono_green, mono_high = render_at(40000), render_at(41000), render_at(42000)
assert 4 not in region(mono_low, *circle_box)
assert 2 not in region(mono_green, *circle_box)
assert set(mono_high[30 * 176 + 3:30 * 176 + 73]) == {0}, \
    "black level did not reach both aperture edges"
assert region(mono_low, *circle_box) != region(mono_green, *circle_box)
assert region(render_at(43999), *circle_box) != region(render_at(44000), *circle_box)
assert region(render_at(44000), 96, 96, 73, 17) == region(render_at(0), 96, 96, 73, 17)

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
assert 6 in region(render_at(14000), *circle_box), "storm lightning is missing"
assert 6 not in region(render_at(14800), *circle_box), "lightning did not blink off"
# The glucose card borrows the circle starting at 16 seconds.
assert 2 in region(render_at(18000), 8, 120, 54, 21)

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
# The restored sun only translates: no ray deformation or changing colors.
sun_reference = render_at(2000)
sun_art = None
positions = set()
for elapsed in range(0, 2400, 50):
    sun = render_at(2000 + elapsed)
    occupied = [(x, y, sun[y * 176 + x]) for y in range(18, 81) for x in range(7, 70)
                if sun[y * 176 + x] != 7]
    left = min(x for x, y, color in occupied)
    art = {(x - left, y, color) for x, y, color in occupied}
    if sun_art is None:
        sun_art = art
    assert art == sun_art, "sun artwork changed during whole-sprite drift"
    positions.add(left)
assert len(positions) == 3 and max(positions) - min(positions) == 2
assert region(sun_reference, *circle_box) == region(render_at(4400), *circle_box), \
    "sun drift must loop seamlessly"
assert 1 in region(render_at(10000), *detail_box), "rain probability must be blue"
assert 1 not in region(render_at(6000), *detail_box), "cloudy label should not imply rain"
assert region(render_at(2000), *middle_box) == region(render_at(22000), *middle_box)
for ms in (2000, 6000, 10000):
    assert set(region(render_at(ms), 146, 42, 23, 7)) == {7}, "DEMO label was not removed"
for ms in (6000, 10000, 14800):
    cloud = region(render_at(ms), 9, 31, 60, 32)
    assert set(cloud).issuperset({0, 1, 3, 7}), "cloud must include outline and colored shadow layers"

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
