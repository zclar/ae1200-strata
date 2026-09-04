CC ?= gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror
BUILD := build
CORE := core/src/strata_display.c core/src/strata_jdi.c

.PHONY: app simulator firmware install-app uninstall-app test clean

$(BUILD)/libstrata_display.so: $(CORE) core/include/strata_display.h core/include/strata_jdi.h
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -fPIC -shared -Icore/include $(CORE) -o $@

app simulator:
	./bin/ae1200-strata

install-app:
	./packaging/install-linux.sh

uninstall-app:
	./packaging/uninstall-linux.sh

firmware:
	west build -p always -b adafruit_itsybitsy/nrf52840 firmware

test: $(BUILD)/libstrata_display.so
	python3 -m py_compile native/simulator.py
	python3 tests/test_renderer.py
	python3 tests/test_faceplate.py

clean:
	rm -f $(BUILD)/libstrata_display.so
