#include "strata_display.h"

#include <stddef.h>

struct scene_info { const char *name; const char *description; };
struct point { int x; int y; };
static const struct scene_info scenes[] = {
    {"Classic AE1200", "An animated Royale-style face with an upper status demo reel."},
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
    if (c == '%') {
        static const uint8_t percent[7] = {0x19, 0x1a, 0x04, 0x08, 0x0b, 0x13, 0x00};
        return percent[row];
    }
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

static void polygon(uint8_t *f, int origin_x, int origin_y,
                    const struct point *points, size_t count, uint8_t color)
{
    int min_x = points[0].x, max_x = points[0].x;
    int min_y = points[0].y, max_y = points[0].y;
    for (size_t i = 1; i < count; ++i) {
        if (points[i].x < min_x) min_x = points[i].x;
        if (points[i].x > max_x) max_x = points[i].x;
        if (points[i].y < min_y) min_y = points[i].y;
        if (points[i].y > max_y) max_y = points[i].y;
    }
    for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
            int inside = 0;
            for (size_t i = 0, previous = count - 1; i < count; previous = i++) {
                const struct point a = points[i], b = points[previous];
                if ((a.y > y) != (b.y > y) &&
                    x < a.x + (y - a.y) * (b.x - a.x) / (b.y - a.y))
                    inside = !inside;
            }
            if (inside) rect(f, origin_x + x, origin_y + y, 1, 1, color);
        }
    }
}

static void bluetooth(uint8_t *f, int x, int y, uint8_t color)
{
    rect(f, x + 3, y, 2, 14, color); rect(f, x + 5, y + 2, 3, 2, color);
    rect(f, x + 6, y + 4, 2, 2, color); rect(f, x + 5, y + 6, 3, 2, color);
    rect(f, x + 6, y + 8, 2, 2, color); rect(f, x + 5, y + 10, 3, 2, color);
    rect(f, x, y + 3, 2, 2, color); rect(f, x + 1, y + 5, 3, 2, color);
    rect(f, x, y + 9, 2, 2, color); rect(f, x + 1, y + 7, 3, 2, color);
}

static void classic_status(uint8_t *f)
{
    label(f, "MUTE", 116, 10, 1, STRATA_BLACK);
    label(f, "ALM SIG", 113, 20, 1, STRATA_BLACK);
    bluetooth(f, 161, 11, STRATA_BLUE);
}

static void battery_status(uint8_t *f, uint32_t reel_elapsed_ms)
{
    const int x = 99, y = 11, width = 68, height = 16;
    unsigned int level = 25u + (reel_elapsed_ms * 70u) / 3999u;
    int fill_width = (int)(level * (unsigned int)(width - 6) / 100u);
    char percentage[] = {'0', '0', '%', '\0'};

    /* Beveled body and terminal preserve the recognizable battery silhouette. */
    line(f, x + 2, y, x + width - 3, y, STRATA_BLACK);
    line(f, x + width - 1, y + 2, x + width - 1, y + height - 3, STRATA_BLACK);
    line(f, x + width - 3, y + height - 1, x + 2, y + height - 1, STRATA_BLACK);
    line(f, x, y + height - 3, x, y + 2, STRATA_BLACK);
    rect(f, x + 1, y + 1, 1, 1, STRATA_BLACK);
    rect(f, x + width - 2, y + 1, 1, 1, STRATA_BLACK);
    rect(f, x + 1, y + height - 2, 1, 1, STRATA_BLACK);
    rect(f, x + width - 2, y + height - 2, 1, 1, STRATA_BLACK);
    rect(f, x + width, y + 5, 4, height - 10, STRATA_BLACK);
    rect(f, x + 3, y + 3, fill_width, height - 6, STRATA_GREEN);

    percentage[0] = (char)('0' + level / 10u);
    percentage[1] = (char)('0' + level % 10u);
    label(f, percentage, 124, 15, 1, STRATA_BLACK);
}

static void status_reel(uint8_t *f, uint32_t elapsed_ms)
{
    const uint32_t item_duration_ms = 4000u;
    uint32_t item = (elapsed_ms / item_duration_ms) % 2u;
    uint32_t item_elapsed_ms = elapsed_ms % item_duration_ms;
    if (item == 0u)
        classic_status(f);
    else
        battery_status(f, item_elapsed_ms);
}

static void analog_clock(uint8_t *f, uint32_t total_seconds)
{
    static const int8_t clock_x[60] = {
        0, 3, 6, 9, 12, 14, 17, 19, 22, 23, 25, 26, 28, 28, 29,
        29, 29, 28, 28, 26, 25, 23, 22, 19, 17, 14, 12, 9, 6, 3,
        0, -3, -6, -9, -12, -14, -17, -19, -22, -23, -25, -26,
        -28, -28, -29, -29, -29, -28, -28, -26, -25, -23, -22,
        -19, -17, -14, -12, -9, -6, -3,
    };
    static const int8_t clock_y[60] = {
        -29, -29, -28, -28, -26, -25, -23, -22, -19, -17, -15,
        -12, -9, -6, -3, 0, 3, 6, 9, 12, 14, 17, 19, 22, 23, 25,
        26, 28, 28, 29, 29, 29, 28, 28, 26, 25, 23, 22, 19, 17,
        15, 12, 9, 6, 3, 0, -3, -6, -9, -12, -15, -17, -19, -22,
        -23, -25, -26, -28, -28, -29,
    };
    const int cx = 38, cy = 49, radius = 32;
    unsigned int second = total_seconds % 60u;
    unsigned int minute = (total_seconds / 60u) % 60u;
    unsigned int hour = ((total_seconds / 3600u) % 12u) * 5u + minute / 12u;
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
    line(f, cx, cy, cx + clock_x[hour] * 15 / 29,
         cy + clock_y[hour] * 15 / 29, STRATA_BLACK);
    line(f, cx, cy, cx + clock_x[minute] * 23 / 29,
         cy + clock_y[minute] * 23 / 29, STRATA_BLACK);
    line(f, cx, cy, cx + clock_x[second], cy + clock_y[second], STRATA_BLACK);
    disc(f, cx, cy, 4, STRATA_BLACK);
}

static void world_map(uint8_t *f)
{
    static const struct point north_america[] = {
        {1, 8}, {5, 4}, {10, 2}, {17, 3}, {20, 5}, {26, 5}, {29, 9},
        {26, 12}, {22, 12}, {20, 16}, {16, 15}, {13, 18}, {10, 15},
        {7, 14}, {4, 12}, {2, 12},
    };
    static const struct point south_america[] = {
        {16, 17}, {21, 17}, {26, 21}, {26, 24}, {23, 28}, {22, 33},
        {19, 37}, {17, 31}, {14, 27}, {14, 22},
    };
    static const struct point greenland[] = {
        {21, 1}, {26, 0}, {30, 3}, {28, 7}, {24, 7},
    };
    static const struct point eurasia[] = {
        {33, 8}, {37, 4}, {42, 3}, {47, 1}, {53, 3}, {58, 2}, {63, 4},
        {70, 6}, {68, 10}, {63, 10}, {60, 13}, {55, 12}, {51, 16},
        {46, 14}, {43, 11}, {38, 12},
    };
    static const struct point africa[] = {
        {39, 15}, {45, 13}, {51, 17}, {51, 21}, {48, 25}, {47, 31},
        {44, 33}, {42, 28}, {39, 24}, {37, 19},
    };
    static const struct point australia[] = {
        {58, 27}, {63, 25}, {69, 27}, {71, 31}, {67, 34}, {61, 34}, {57, 31},
    };

    /* The stock map has dotted latitude/longitude guides and a solid equator. */
    for (int y = 48; y <= 72; y += 24)
        for (int x = 99; x <= 170; x += 3) rect(f, x, y, 1, 1, STRATA_BLACK);
    for (int x = 111; x <= 159; x += 24)
        for (int y = 40; y <= 80; y += 3) rect(f, x, y, 1, 1, STRATA_BLACK);
    polygon(f, 99, 41, north_america, sizeof(north_america) / sizeof(north_america[0]), STRATA_BLACK);
    polygon(f, 99, 41, south_america, sizeof(south_america) / sizeof(south_america[0]), STRATA_BLACK);
    polygon(f, 99, 41, greenland, sizeof(greenland) / sizeof(greenland[0]), STRATA_BLACK);
    polygon(f, 99, 41, eurasia, sizeof(eurasia) / sizeof(eurasia[0]), STRATA_BLACK);
    polygon(f, 99, 41, africa, sizeof(africa) / sizeof(africa[0]), STRATA_BLACK);
    polygon(f, 99, 41, australia, sizeof(australia) / sizeof(australia[0]), STRATA_BLACK);
    rect(f, 132, 48, 2, 2, STRATA_BLACK); /* United Kingdom */
    rect(f, 153, 55, 2, 3, STRATA_BLACK); /* Japan */
    rect(f, 152, 72, 1, 3, STRATA_BLACK); /* Madagascar */
    rect(f, 155, 66, 2, 2, STRATA_BLACK); /* Indonesia */
    line(f, 99, 60, 170, 60, STRATA_BLACK);

    /* Hollow UTC-5 marker keeps the selected zone visible without hiding the map. */
    line(f, 117, 40, 123, 40, STRATA_BLUE);
    line(f, 123, 40, 123, 79, STRATA_BLUE);
    line(f, 123, 79, 117, 79, STRATA_BLUE);
    line(f, 117, 79, 117, 40, STRATA_BLUE);
}

static void main_time(uint8_t *f, uint32_t total_seconds, uint32_t elapsed_ms)
{
    unsigned int second = total_seconds % 60u;
    unsigned int minute = (total_seconds / 60u) % 60u;
    unsigned int hour = (total_seconds / 3600u) % 12u;
    if (hour == 0u) hour = 12u;

    label(f, "SUN", 103, 100, 1, STRATA_BLACK);
    label(f, "6-30", 135, 100, 1, STRATA_BLACK);
    line(f, 130, 96, 130, 113, STRATA_BLACK);
    line(f, 82, 115, 168, 115, STRATA_BLACK);
    label(f, "DST", 139, 123, 1, STRATA_BLACK);
    label(f, "PM", 6, 134, 1, STRATA_BLACK);

    if (hour >= 10u) seven_digit(f, hour / 10u, 30, 122, 0, STRATA_BLACK);
    seven_digit(f, hour % 10u, 52, 122, 0, STRATA_BLACK);
    if ((elapsed_ms / 500u) % 2u == 0u) {
        disc(f, 77, 135, 2, STRATA_BLACK);
        disc(f, 77, 150, 2, STRATA_BLACK);
    }
    seven_digit(f, minute / 10u, 83, 122, 0, STRATA_BLACK);
    seven_digit(f, minute % 10u, 105, 122, 0, STRATA_BLACK);
    seven_digit(f, second / 10u, 135, 140, 1, STRATA_BLACK);
    seven_digit(f, second % 10u, 148, 140, 1, STRATA_BLACK);
}

static void segment_face(uint8_t *f, uint32_t elapsed_ms)
{
    uint32_t total_seconds = 10u * 3600u + 8u * 60u + 36u + elapsed_ms / 1000u;
    /* Original AE-1200 timekeeping layout, fitted to the four cover openings. */
    analog_clock(f, total_seconds);
    status_reel(f, elapsed_ms);
    world_map(f);
    main_time(f, total_seconds, elapsed_ms);
}

unsigned int strata_scene_count(void) { return (unsigned int)(sizeof(scenes) / sizeof(scenes[0])); }
const char *strata_scene_name(unsigned int scene) { return scenes[scene % strata_scene_count()].name; }
const char *strata_scene_description(unsigned int scene) { return scenes[scene % strata_scene_count()].description; }

void strata_render(uint8_t *frame, unsigned int scene, uint32_t elapsed_ms)
{
    (void)scene;
    if (!frame) return;
    fill(frame, STRATA_WHITE);
    segment_face(frame, elapsed_ms);
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
