#include "lpm013m126a.h"

#include <errno.h>

#include <zephyr/sys/util.h>

#include "strata_jdi.h"

#define SCS_LOW_US 6u
#define SCS_SETUP_US 6u
#define SCS_HOLD_US 2u
#define POWER_CLEAR_MS 1u
#define LATCH_RELEASE_US 30u
#define DISPLAY_COM_START_US 100u

static void extcomin_toggle(struct k_timer *timer)
{
	struct strata_lpm013m126a *panel =
		CONTAINER_OF(timer, struct strata_lpm013m126a, extcomin_timer);

	panel->extcomin_high = !panel->extcomin_high;
	(void)gpio_pin_set_dt(&panel->extcomin, panel->extcomin_high);
}

static int transfer(struct strata_lpm013m126a *panel, const uint8_t *data, size_t length)
{
	struct spi_buf buffer = {.buf = (void *)data, .len = length};
	struct spi_buf_set buffers = {.buffers = &buffer, .count = 1};
	int result;

	k_busy_wait(SCS_LOW_US);
	result = gpio_pin_set_dt(&panel->scs, 1);
	if (result < 0) return result;
	k_busy_wait(SCS_SETUP_US);

	result = spi_write_dt(&panel->bus, &buffers);
	k_busy_wait(SCS_HOLD_US);
	if (gpio_pin_set_dt(&panel->scs, 0) < 0 && result == 0) result = -EIO;
	return result;
}

static int all_clear(struct strata_lpm013m126a *panel)
{
	uint8_t packet[STRATA_JDI_CLEAR_PACKET_BYTES];
	int length = strata_jdi_encode_all_clear(packet, sizeof(packet));
	if (length < 0) return -EINVAL;
	return transfer(panel, packet, (size_t)length);
}

int strata_lpm013m126a_init(struct strata_lpm013m126a *panel)
{
	int result;

	if (!panel || panel->extcomin_frequency == 0u) return -EINVAL;
	if (!spi_is_ready_dt(&panel->bus) || !gpio_is_ready_dt(&panel->scs) ||
	    !gpio_is_ready_dt(&panel->disp) || !gpio_is_ready_dt(&panel->extcomin)) {
		return -ENODEV;
	}

	result = gpio_pin_configure_dt(&panel->scs, GPIO_OUTPUT_INACTIVE);
	if (result < 0) return result;
	result = gpio_pin_configure_dt(&panel->disp, GPIO_OUTPUT_INACTIVE);
	if (result < 0) return result;
	result = gpio_pin_configure_dt(&panel->extcomin, GPIO_OUTPUT_INACTIVE);
	if (result < 0) return result;

	result = all_clear(panel);
	if (result < 0) return result;
	k_msleep(POWER_CLEAR_MS);
	k_busy_wait(LATCH_RELEASE_US);

	k_timer_init(&panel->extcomin_timer, extcomin_toggle, NULL);
	panel->extcomin_high = true;
	result = gpio_pin_set_dt(&panel->disp, 1);
	if (result < 0) return result;
	result = gpio_pin_set_dt(&panel->extcomin, 1);
	if (result < 0) {
		(void)gpio_pin_set_dt(&panel->disp, 0);
		return result;
	}
	k_busy_wait(DISPLAY_COM_START_US);

	uint32_t half_period_ms = 500u / panel->extcomin_frequency;
	if (half_period_ms == 0u) half_period_ms = 1u;
	k_timer_start(&panel->extcomin_timer, K_MSEC(half_period_ms), K_MSEC(half_period_ms));
	panel->initialized = true;
	return 0;
}

int strata_lpm013m126a_write_frame(struct strata_lpm013m126a *panel,
				   const uint8_t *frame)
{
	uint8_t packet[STRATA_JDI_LINE_PACKET_BYTES];

	if (!panel || !frame || !panel->initialized) return -EINVAL;
	for (unsigned int line = 0; line < STRATA_HEIGHT; ++line) {
		int length = strata_jdi_encode_line(frame, line, packet, sizeof(packet));
		if (length < 0) return -EINVAL;
		int result = transfer(panel, packet, (size_t)length);
		if (result < 0) return result;
	}
	return 0;
}

int strata_lpm013m126a_shutdown(struct strata_lpm013m126a *panel)
{
	int result;

	if (!panel || !panel->initialized) return -EINVAL;
	result = all_clear(panel);
	k_msleep(POWER_CLEAR_MS);
	k_timer_stop(&panel->extcomin_timer);
	(void)gpio_pin_set_dt(&panel->extcomin, 0);
	(void)gpio_pin_set_dt(&panel->disp, 0);
	panel->initialized = false;
	return result;
}
