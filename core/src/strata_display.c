#include "strata_display.h"

#include <stddef.h>

struct scene_info { const char *name; const char *description; };
static const struct scene_info scenes[] = {
    {"World time", "Local time and global context."},
    {"Activity", "Daily movement and recovery."},
    {"Weather", "Current conditions and forecast."},
    {"Navigation", "Glanceable turn guidance."},
    {"Notification", "A quiet message preview."},
    {"Timer", "High-contrast focus timing."},
    {"Segment face", "A pixel-first face fitted to every cover opening."},
    {"Segment dashboard", "Chunky status graphics composed around the mask."},
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

static void rule(uint8_t *f, int y) { rect(f, 8, y, 160, 1, STRATA_WHITE); }

static void top(uint8_t *f, const char *left, const char *right)
{
    rect(f, 12, 13, 51, 43, STRATA_GREEN);
    rect(f, 16, 17, 43, 35, STRATA_BLACK);
    label(f, left, 18, 28, 2, STRATA_WHITE);
    label(f, right, 88, 25, 3, STRATA_CYAN);
    rule(f, 72);
}

static void world(uint8_t *f)
{
    top(f, "BAT", "NYC"); label(f, "MON 18", 10, 81, 2, STRATA_WHITE);
    label(f, "10:09", 27, 104, 8, STRATA_WHITE); label(f, "NEW YORK", 48, 151, 2, STRATA_CYAN);
}

static void activity(uint8_t *f)
{
    top(f, "MOVE", "84%"); label(f, "TODAY", 10, 81, 2, STRATA_WHITE);
    rect(f, 14, 106, 18, 52, STRATA_WHITE); rect(f, 38, 121, 18, 37, STRATA_GREEN); rect(f, 62, 136, 18, 22, STRATA_YELLOW);
    label(f, "8421", 96, 105, 4, STRATA_WHITE); label(f, "STEPS", 96, 129, 2, STRATA_CYAN); label(f, "5.8 KM", 96, 146, 2, STRATA_WHITE);
}

static void weather(uint8_t *f)
{
    top(f, "UV3", "BOS"); label(f, "FORECAST", 10, 81, 2, STRATA_WHITE);
    rect(f, 20, 108, 34, 34, STRATA_YELLOW); rect(f, 28, 100, 18, 50, STRATA_YELLOW); rect(f, 12, 116, 50, 18, STRATA_YELLOW);
    label(f, "68", 86, 102, 7, STRATA_WHITE); label(f, "CLEAR", 87, 140, 2, STRATA_CYAN); label(f, "H72 L54", 87, 156, 2, STRATA_WHITE);
}

static void navigation(uint8_t *f)
{
    top(f, "N", "0.4MI"); label(f, "WALK", 10, 81, 2, STRATA_WHITE);
    rect(f, 25, 111, 7, 48, STRATA_CYAN); rect(f, 25, 106, 42, 7, STRATA_CYAN); rect(f, 60, 97, 7, 16, STRATA_CYAN);
    rect(f, 55, 97, 17, 7, STRATA_CYAN); label(f, "TURN", 87, 105, 3, STRATA_WHITE); label(f, "RIGHT", 87, 124, 3, STRATA_WHITE); label(f, "6 MIN", 87, 151, 2, STRATA_GREEN);
}

static void notification(uint8_t *f)
{
    top(f, "MSG", "1 NEW"); label(f, "PHONE", 10, 81, 2, STRATA_WHITE);
    rect(f, 10, 101, 5, 62, STRATA_CYAN); label(f, "ALEX", 25, 101, 3, STRATA_CYAN);
    label(f, "PROTOTYPE", 25, 124, 2, STRATA_WHITE); label(f, "LOOKS GREAT", 25, 141, 2, STRATA_WHITE); label(f, "10:09", 25, 158, 1, STRATA_WHITE);
}

static void timer(uint8_t *f, uint32_t elapsed_ms)
{
    unsigned int seconds = 37u + elapsed_ms / 1000u;
    unsigned int remaining = seconds < 25u * 60u ? 25u * 60u - seconds : 0;
    char value[] = "00:00";
    value[0] = (char)('0' + (remaining / 600u) % 10u); value[1] = (char)('0' + (remaining / 60u) % 10u);
    value[3] = (char)('0' + (remaining / 10u) % 6u); value[4] = (char)('0' + remaining % 10u);
    top(f, "TMR", "RUN"); label(f, "FOCUS", 10, 81, 2, STRATA_WHITE);
    label(f, value, 27, 108, 8, STRATA_WHITE); rect(f, 15, 153, 146, 7, STRATA_WHITE); rect(f, 15, 153, 112, 7, STRATA_GREEN);
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
    disc(f, 40, 49, 33, STRATA_GREEN); disc(f, 40, 49, 29, STRATA_BLACK);
    rect(f, 39, 17, 2, 10, STRATA_WHITE); rect(f, 39, 71, 2, 10, STRATA_WHITE);
    rect(f, 8, 48, 10, 2, STRATA_WHITE); rect(f, 62, 48, 10, 2, STRATA_WHITE);
    label(f, "12", 35, 20, 2, STRATA_WHITE); label(f, "6", 37, 68, 2, STRATA_WHITE);
    rect(f, 40, 27, 3, 23, STRATA_CYAN); rect(f, 40, 48, 22, 3, STRATA_CYAN);
    disc(f, 40, 49, 4, STRATA_YELLOW);
    label(f, "WR", 105, 12, 2, STRATA_WHITE);
    rect(f, 101, 42, 9, 10, STRATA_GREEN); rect(f, 111, 38, 12, 14, STRATA_GREEN);
    rect(f, 124, 46, 13, 8, STRATA_GREEN); rect(f, 138, 40, 11, 16, STRATA_GREEN);
    rect(f, 151, 50, 18, 9, STRATA_GREEN); rect(f, 116, 58, 9, 13, STRATA_GREEN);
    label(f, "CITY", 105, 70, 2, STRATA_CYAN);
    label(f, "10:09", 19, 105, 7, STRATA_WHITE); label(f, "MON 18", 46, 151, 2, STRATA_CYAN);
    bluetooth(f, 164, 96, STRATA_CYAN);
}

static void segment_dashboard(uint8_t *f)
{
    disc(f, 40, 49, 33, STRATA_BLUE); disc(f, 40, 49, 29, STRATA_BLACK);
    rect(f, 38, 19, 5, 16, STRATA_WHITE); rect(f, 38, 63, 5, 16, STRATA_WHITE);
    rect(f, 10, 47, 16, 5, STRATA_WHITE); rect(f, 54, 47, 16, 5, STRATA_WHITE);
    rect(f, 40, 32, 3, 25, STRATA_YELLOW); rect(f, 40, 48, 19, 3, STRATA_YELLOW);
    disc(f, 40, 49, 3, STRATA_RED);
    label(f, "84%", 103, 12, 2, STRATA_YELLOW); label(f, "BOS", 106, 43, 2, STRATA_CYAN);
    label(f, "RUN", 106, 66, 2, STRATA_WHITE);
    label(f, "42%", 40, 104, 8, STRATA_WHITE);
    rect(f, 16, 153, 144, 6, STRATA_WHITE); rect(f, 16, 153, 91, 6, STRATA_GREEN);
    label(f, "BATTERY", 16, 161, 1, STRATA_CYAN);
}

unsigned int strata_scene_count(void) { return (unsigned int)(sizeof(scenes) / sizeof(scenes[0])); }
const char *strata_scene_name(unsigned int scene) { return scenes[scene % strata_scene_count()].name; }
const char *strata_scene_description(unsigned int scene) { return scenes[scene % strata_scene_count()].description; }

void strata_render(uint8_t *frame, unsigned int scene, uint32_t elapsed_ms)
{
    if (!frame) return;
    fill(frame, STRATA_BLACK);
    switch (scene % strata_scene_count()) {
    case 0: world(frame); break; case 1: activity(frame); break;
    case 2: weather(frame); break; case 3: navigation(frame); break;
    case 4: notification(frame); break; case 5: timer(frame, elapsed_ms); break;
    case 6: segment_face(frame); break; default: segment_dashboard(frame); break;
    }
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
