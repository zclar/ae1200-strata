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
	uint32_t last_report_ms = 0;
	uint32_t frames = 0;

	result = strata_lpm013m126a_init(&panel);
	if (result < 0) {
		printk("LPM013M126A initialization failed: %d\n", result);
		return result;
	}

	printk("AE1200 animated classic face started\n");
	started = k_uptime_get_32();
	for (;;) {
		uint32_t elapsed_ms = k_uptime_get_32() - started;
		strata_render(framebuffer, 0, elapsed_ms);
		result = strata_lpm013m126a_write_frame(&panel, framebuffer);
		if (result < 0) {
			printk("LPM013M126A frame write failed: %d\n", result);
			return result;
		}
		++frames;
		if (elapsed_ms - last_report_ms >= 4000u) {
			printk("AE1200 demo: elapsed=%u ms, frames=%u, RGB111 white background\n",
			       (unsigned int)elapsed_ms, (unsigned int)frames);
			last_report_ms = elapsed_ms;
		}
		k_sleep(K_MSEC(20));
	}
}
