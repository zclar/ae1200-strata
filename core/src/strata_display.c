#include "strata_display.h"

#include <stddef.h>

struct scene_info { const char *name; const char *description; };
static const struct scene_info scenes[] = {
    {"Classic AE1200", "A monochrome Royale-style face fitted to the cover."},
};

/* Readable 5x7 glyphs for the panel's small LCD-style legends. */
static uint8_t glyph_row(char c, int row)
{
    static const uint8_t digits[10][7] = {
        {0x0e, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0e},
        {0x04, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x0e},
        {0x0e, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1f},
        {0x1e, 0x01, 0x01, 0x0e, 0x01, 0x01, 0x1e},
        {0x02, 0x06, 0x0a, 0x12, 0x1f, 0x02, 0x02},
        {0x1f, 0x10, 0x10, 0x1e, 0x01, 0x01, 0x1e},
        {0x0e, 0x10, 0x10, 0x1e, 0x11, 0x11, 0x0e},
        {0x1f, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08},
        {0x0e, 0x11, 0x11, 0x0e, 0x11, 0x11, 0x0e},
        {0x0e, 0x11, 0x11, 0x0f, 0x01, 0x01, 0x0e},
    };
    static const uint8_t letters[26][7] = {
        {0x0e, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11},
        {0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e},
        {0x0e, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0e},
        {0x1e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1e},
        {0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x1f},
        {0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x10},
        {0x0e, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0f},
        {0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11},
        {0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e},
        {0x07, 0x02, 0x02, 0x02, 0x12, 0x12, 0x0c},
        {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11},
        {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f},
        {0x11, 0x1b, 0x15, 0x15, 0x11, 0x11, 0x11},
        {0x11, 0x19, 0x19, 0x15, 0x13, 0x13, 0x11},
        {0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e},
        {0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10},
        {0x0e, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0d},
        {0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11},
        {0x0f, 0x10, 0x10, 0x0e, 0x01, 0x01, 0x1e},
        {0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},
        {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e},
        {0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04},
        {0x11, 0x11, 0x11, 0x15, 0x15, 0x1b, 0x11},
        {0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11},
        {0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04},
        {0x1f, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1f},
    };
    if (c >= '0' && c <= '9') return digits[c - '0'][row];
    if (c >= 'A' && c <= 'Z') return letters[c - 'A'][row];
    if (c == ':' && (row == 2 || row == 5)) return 0x04;
    if (c == '.' && row == 6) return 0x04;
    if (c == '-' && row == 3) return 0x0e;
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
        char c = *s++;
        for (int row = 0; row < 7; ++row)
            for (int col = 0; col < 5; ++col)
                if (glyph_row(c, row) & (1u << (4 - col)))
                    rect(f, x + col * scale, y + row * scale, scale, scale, color);
        x += 6 * scale;
    }
}

static void line(uint8_t *f, int x0, int y0, int x1, int y1, uint8_t color)
{
    int dx = x1 > x0 ? x1 - x0 : x0 - x1;
    int sx = x0 < x1 ? 1 : -1;
    int dy = y1 > y0 ? y0 - y1 : y1 - y0;
    int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;
    for (;;) {
        rect(f, x0, y0, 1, 1, color);
        if (x0 == x1 && y0 == y1) break;
        int twice = error * 2;
        if (twice >= dy) { error += dy; x0 += sx; }
        if (twice <= dx) { error += dx; y0 += sy; }
    }
}

static void horizontal_segment(uint8_t *f, int x, int y, int width,
                               int thickness, uint8_t color)
{
    for (int row = 0; row < thickness; ++row) {
        int inset = row == thickness / 2 ? 0 : 1;
        rect(f, x + inset, y + row, width - inset * 2, 1, color);
    }
}

static void vertical_segment(uint8_t *f, int x, int y, int height,
                             int thickness, uint8_t color)
{
    for (int column = 0; column < thickness; ++column) {
        int inset = column == thickness / 2 ? 0 : 1;
        rect(f, x + column, y + inset, 1, height - inset * 2, color);
    }
}

static void seven_digit(uint8_t *f, unsigned int digit, int x, int y,
                        int small, uint8_t color)
{
    static const uint8_t segments[] = {
        0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f,
    };
    uint8_t on = segments[digit % 10u];
    int thickness = small ? 2 : 3;
    int width = small ? 9 : 19;
    int vertical_height = small ? 10 : 18;
    int right = small ? 9 : 18;
    int middle = small ? 9 : 18;
    int lower = small ? 10 : 20;
    int bottom = small ? 19 : 38;
    if (on & 0x01) horizontal_segment(f, x + 1, y, width, thickness, color);
    if (on & 0x02) vertical_segment(f, x + right, y + 1, vertical_height, thickness, color);
    if (on & 0x04) vertical_segment(f, x + right, y + lower, vertical_height, thickness, color);
    if (on & 0x08) horizontal_segment(f, x + 1, y + bottom, width, thickness, color);
    if (on & 0x10) vertical_segment(f, x, y + lower, vertical_height, thickness, color);
    if (on & 0x20) vertical_segment(f, x, y + 1, vertical_height, thickness, color);
    if (on & 0x40) horizontal_segment(f, x + 1, y + middle, width, thickness, color);
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

static void analog_clock(uint8_t *f)
{
    const int cx = 40, cy = 49, radius = 32;
    disc(f, cx, cy, radius, STRATA_WHITE);
    for (int y = -radius; y <= radius; ++y) {
        for (int x = -radius; x <= radius; ++x) {
            int distance = x * x + y * y;
            if (distance <= radius * radius && distance >= (radius - 2) * (radius - 2))
                rect(f, cx + x, cy + y, 1, 1, STRATA_BLACK);
        }
    }
    line(f, cx, cy - 29, cx, cy - 7, STRATA_BLACK);
    line(f, cx + 29, cy, cx + 7, cy, STRATA_BLACK);
    line(f, cx, cy + 29, cx, cy + 7, STRATA_BLACK);
    line(f, cx - 29, cy, cx - 7, cy, STRATA_BLACK);
    line(f, cx, cy, cx - 17, cy - 10, STRATA_BLACK);
    line(f, cx, cy, cx + 17, cy - 21, STRATA_BLACK);
    disc(f, cx, cy, 4, STRATA_BLACK);
}

static void world_map(uint8_t *f)
{
    static const char *const rows[] = {
        "      #####         #######",
        "    ########      ############",
        "   ##########   ################",
        "  ##########   #################",
        "  #########   ##################",
        "    ######     ###############",
        "     #####       ###########",
        "     ####          #######",
        "      ###           #####",
        "      ####           ####",
        "       ###           ####",
        "       ###           ###",
        "        ###          ###",
        "         ##          ##",
        "         ##              ####",
        "                         #####",
        "                          ###",
    };
    for (int y = 42; y <= 78; y += 12)
        for (int x = 101; x <= 172; x += 3) rect(f, x, y, 1, 1, STRATA_BLACK);
    for (int x = 113; x <= 161; x += 24)
        for (int y = 40; y <= 80; y += 3) rect(f, x, y, 1, 1, STRATA_BLACK);
    for (size_t row = 0; row < sizeof(rows) / sizeof(rows[0]); ++row)
        for (size_t column = 0; rows[row][column]; ++column)
            if (rows[row][column] == '#')
                rect(f, 101 + (int)column * 2, 41 + (int)row * 2, 2, 2, STRATA_BLACK);
}

static void main_time(uint8_t *f)
{
    label(f, "SUN", 100, 99, 1, STRATA_BLACK);
    label(f, "6-30", 139, 99, 1, STRATA_BLACK);
    label(f, "PM", 8, 134, 1, STRATA_BLACK);

    seven_digit(f, 1, 32, 122, 0, STRATA_BLACK);
    seven_digit(f, 0, 54, 122, 0, STRATA_BLACK);
    disc(f, 79, 135, 2, STRATA_BLACK);
    disc(f, 79, 150, 2, STRATA_BLACK);
    seven_digit(f, 0, 85, 122, 0, STRATA_BLACK);
    seven_digit(f, 8, 107, 122, 0, STRATA_BLACK);
    seven_digit(f, 3, 137, 140, 1, STRATA_BLACK);
    seven_digit(f, 6, 150, 140, 1, STRATA_BLACK);
}

static void segment_face(uint8_t *f)
{
    /* Original AE-1200 timekeeping layout, fitted to the four cover openings. */
    analog_clock(f);
    label(f, "MUTE", 118, 10, 1, STRATA_BLACK);
    label(f, "ALM SIG", 115, 20, 1, STRATA_BLACK);
    bluetooth(f, 163, 11, STRATA_BLACK);
    world_map(f);
    main_time(f);
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
