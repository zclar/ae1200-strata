#ifndef STRATA_DISPLAY_H
#define STRATA_DISPLAY_H

#include <stdint.h>

#define STRATA_WIDTH 176
#define STRATA_HEIGHT 176
#define STRATA_FRAME_BYTES (STRATA_WIDTH * STRATA_HEIGHT)

enum strata_color {
    STRATA_BLACK = 0,
    STRATA_BLUE = 1,
    STRATA_GREEN = 2,
    STRATA_CYAN = 3,
    STRATA_RED = 4,
    STRATA_MAGENTA = 5,
    STRATA_YELLOW = 6,
    STRATA_WHITE = 7,
};

unsigned int strata_scene_count(void);
const char *strata_scene_name(unsigned int scene);
const char *strata_scene_description(unsigned int scene);
void strata_render(uint8_t *frame, unsigned int scene, uint32_t elapsed_ms);

#endif
