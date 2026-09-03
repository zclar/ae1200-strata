#include "strata_display.h"

#include <stddef.h>

struct scene_info { const char *name; const char *description; };
static const struct scene_info scenes[] = {
    {"Classic AE1200", "A monochrome Royale-style face fitted to the cover."},
};

/* 3x5 glyphs, encoded left-to-right in each group of three bits. */
static uint16_t glyph(char c)
{
    static const uint16_t digits[] = {
        0x7B6F, 0x2492, 0x73E7, 0x73CF, 0x5BC9,
        0x79CF, 0x79EF, 0x7249, 0x7BEF, 0x7BCF,
    };
    static const uint16_t letters[] = {
        0x7BED, 0x6BAE, 0x7927, 0x6B6E, 0x79E7, 0x79E4, 0x79AF,
        0x5BED, 0x7497, 0x124F, 0x5AAD, 0x4927, 0x5FED, 0x5F6D,
        0x7B6F, 0x7BE4, 0x7B7B, 0x7BAC, 0x79CF, 0x7492, 0x5B6F,
        0x5B6A, 0x5FFD, 0x5AAD, 0x5A92, 0x72A7,
    };
    if (c >= '0' && c <= '9') return digits[c - '0'];
    if (c >= 'A' && c <= 'Z') return letters[c - 'A'];
    if (c == ':') return 0x0410;
    if (c == '.') return 0x0002;
    if (c == '-') return 0x01C0;
    if (c == '%') return 0x5295;
    return 0;
}

static void fill(uint8_t *f, uint8_t color)
{
    for (size_t i = 0; i < STRATA_FRAME_BYTES; ++i) f[i] = color;
}

static void rect(uint8_t *f, int x, int y, int w, int h, uint8_t color)
{
    for (int py = y; py < y + h; ++py)
        for (int px = x; px < x + w; ++px)
            if (px >= 0 && px < STRATA_WIDTH && py >= 0 && py < STRATA_HEIGHT)
                f[py * STRATA_WIDTH + px] = color;
}

static void label(uint8_t *f, const char *s, int x, int y, int scale, uint8_t color)
{
    while (*s) {
        uint16_t bits = glyph(*s++);
        for (int row = 0; row < 5; ++row)
            for (int col = 0; col < 3; ++col)
                if (bits & (1u << (14 - row * 3 - col)))
                    rect(f, x + col * scale, y + row * scale, scale, scale, color);
        x += 4 * scale;
    }
}

static void seven_digit(uint8_t *f, unsigned int digit, int x, int y, uint8_t color)
{
    static const uint8_t segments[] = {
        0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f,
    };
    uint8_t on = segments[digit % 10u];
    if (on & 0x01) rect(f, x + 3, y, 14, 3, color);
    if (on & 0x02) rect(f, x + 17, y + 3, 3, 13, color);
    if (on & 0x04) rect(f, x + 17, y + 18, 3, 13, color);
    if (on & 0x08) rect(f, x + 3, y + 31, 14, 3, color);
    if (on & 0x10) rect(f, x, y + 18, 3, 13, color);
    if (on & 0x20) rect(f, x, y + 3, 3, 13, color);
    if (on & 0x40) rect(f, x + 3, y + 15, 14, 3, color);
}

static void classic_time(uint8_t *f, int x, int y, uint8_t color)
{
    seven_digit(f, 1, x, y, color); seven_digit(f, 0, x + 24, y, color);
    rect(f, x + 47, y + 10, 3, 3, color); rect(f, x + 47, y + 22, 3, 3, color);
    seven_digit(f, 0, x + 55, y, color); seven_digit(f, 9, x + 79, y, color);
}

static void disc(uint8_t *f, int cx, int cy, int radius, uint8_t color)
{
    for (int y = -radius; y <= radius; ++y)
        for (int x = -radius; x <= radius; ++x)
            if (x * x + y * y <= radius * radius)
                rect(f, cx + x, cy + y, 1, 1, color);
}

static void bluetooth(uint8_t *f, int x, int y, uint8_t color)
{
    rect(f, x + 3, y, 2, 14, color); rect(f, x + 5, y + 2, 3, 2, color);
    rect(f, x + 6, y + 4, 2, 2, color); rect(f, x + 5, y + 6, 3, 2, color);
    rect(f, x + 6, y + 8, 2, 2, color); rect(f, x + 5, y + 10, 3, 2, color);
    rect(f, x, y + 3, 2, 2, color); rect(f, x + 1, y + 5, 3, 2, color);
    rect(f, x, y + 9, 2, 2, color); rect(f, x + 1, y + 7, 3, 2, color);
}

static void segment_face(uint8_t *f)
{
    /* Classic AE-1200-inspired face, fitted to the measured openings. */
    disc(f, 40, 49, 27, STRATA_BLACK); disc(f, 40, 49, 24, STRATA_WHITE);
    rect(f, 39, 23, 2, 7, STRATA_BLACK); rect(f, 39, 68, 2, 7, STRATA_BLACK);
    rect(f, 14, 48, 7, 2, STRATA_BLACK); rect(f, 59, 48, 7, 2, STRATA_BLACK);
    label(f, "12", 35, 25, 1, STRATA_BLACK); label(f, "6", 38, 69, 1, STRATA_BLACK);
    rect(f, 40, 31, 2, 19, STRATA_BLACK); rect(f, 40, 48, 17, 2, STRATA_BLACK);
    disc(f, 40, 49, 3, STRATA_BLACK);

    label(f, "WR", 111, 14, 1, STRATA_BLACK);
    rect(f, 105, 44, 7, 8, STRATA_BLACK); rect(f, 113, 41, 10, 11, STRATA_BLACK);
    rect(f, 125, 47, 11, 7, STRATA_BLACK); rect(f, 138, 43, 9, 13, STRATA_BLACK);
    rect(f, 149, 51, 16, 7, STRATA_BLACK); rect(f, 117, 59, 8, 10, STRATA_BLACK);
    label(f, "CITY", 112, 72, 1, STRATA_BLACK);

    classic_time(f, 37, 120, STRATA_BLACK);
    label(f, "MON 18", 66, 158, 1, STRATA_BLACK);
    bluetooth(f, 158, 99, STRATA_BLACK);
}

unsigned int strata_scene_count(void) { return (unsigned int)(sizeof(scenes) / sizeof(scenes[0])); }
const char *strata_scene_name(unsigned int scene) { return scenes[scene % strata_scene_count()].name; }
const char *strata_scene_description(unsigned int scene) { return scenes[scene % strata_scene_count()].description; }

void strata_render(uint8_t *frame, unsigned int scene, uint32_t elapsed_ms)
{
    (void)scene;
    (void)elapsed_ms;
    if (!frame) return;
    fill(frame, STRATA_WHITE);
    segment_face(frame);
}

void strata_pack_line_rgb111(const uint8_t *frame, unsigned int line,
                             uint8_t *packed_line)
{
    if (!frame || !packed_line || line >= STRATA_HEIGHT) return;
    const uint8_t *pixels = frame + line * STRATA_WIDTH;
    for (unsigned int x = 0; x < STRATA_WIDTH; x += 2) {
        /* JDI 4-bit color update format is RGB0, most-significant pixel first. */
        uint8_t first = (uint8_t)((pixels[x] & 0x07u) << 1);
        uint8_t second = (uint8_t)((pixels[x + 1] & 0x07u) << 1);
        packed_line[x / 2] = (uint8_t)((first << 4) | second);
    }
}
