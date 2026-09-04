#ifndef STRATA_LPM013M126A_H
#define STRATA_LPM013M126A_H

#include <stdbool.h>
#include <stdint.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/kernel.h>

struct strata_lpm013m126a {
	struct spi_dt_spec bus;
	struct gpio_dt_spec scs;
	struct gpio_dt_spec disp;
	struct gpio_dt_spec extcomin;
	struct k_timer extcomin_timer;
	uint32_t extcomin_frequency;
	bool extcomin_high;
	bool initialized;
};

#define STRATA_LPM013M126A_DT_SPEC_GET(node_id)                                      \
	{                                                                              \
		.bus = SPI_DT_SPEC_GET(node_id, SPI_OP_MODE_CONTROLLER | SPI_WORD_SET(8) | \
						  SPI_TRANSFER_MSB, 0),                       \
		.scs = GPIO_DT_SPEC_GET(node_id, scs_gpios),                                \
		.disp = GPIO_DT_SPEC_GET(node_id, disp_gpios),                              \
		.extcomin = GPIO_DT_SPEC_GET(node_id, extcomin_gpios),                      \
		.extcomin_frequency = DT_PROP(node_id, extcomin_frequency),                 \
	}

int strata_lpm013m126a_init(struct strata_lpm013m126a *panel);
int strata_lpm013m126a_write_frame(struct strata_lpm013m126a *panel,
				   const uint8_t *frame);
int strata_lpm013m126a_shutdown(struct strata_lpm013m126a *panel);

#endif
