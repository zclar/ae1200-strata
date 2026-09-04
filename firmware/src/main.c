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
	uint32_t started;

	result = strata_lpm013m126a_init(&panel);
	if (result < 0) {
		printk("LPM013M126A initialization failed: %d\n", result);
		return result;
	}

	printk("AE1200 animated classic face started\n");
	started = k_uptime_get_32();
	for (;;) {
		strata_render(framebuffer, 0, k_uptime_get_32() - started);
		result = strata_lpm013m126a_write_frame(&panel, framebuffer);
		if (result < 0) {
			printk("LPM013M126A frame write failed: %d\n", result);
			return result;
		}
		k_sleep(K_MSEC(500));
	}
}
