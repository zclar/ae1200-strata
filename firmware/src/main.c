#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "lpm013m126a.h"
#include "strata_display.h"

#define PANEL_NODE DT_NODELABEL(strata_panel)

static struct strata_lpm013m126a panel = STRATA_LPM013M126A_DT_SPEC_GET(PANEL_NODE);
static uint8_t framebuffer[STRATA_FRAME_BYTES];

int main(void)
{
	int result;

	strata_render(framebuffer, 0, 0);
	result = strata_lpm013m126a_init(&panel);
	if (result < 0) {
		printk("LPM013M126A initialization failed: %d\n", result);
		return result;
	}

	result = strata_lpm013m126a_write_frame(&panel, framebuffer);
	if (result < 0) {
		printk("LPM013M126A frame write failed: %d\n", result);
		return result;
	}
	printk("AE1200 classic face written to LPM013M126A\n");

	for (;;) k_sleep(K_FOREVER);
}
