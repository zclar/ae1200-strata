CC ?= gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror
BUILD := build
CORE := core/src/strata_display.c

.PHONY: app simulator install-app uninstall-app test clean

$(BUILD)/libstrata_display.so: $(CORE) core/include/strata_display.h
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -fPIC -shared -Icore/include $(CORE) -o $@

app simulator:
	./bin/ae1200-strata

install-app:
	./packaging/install-linux.sh

uninstall-app:
	./packaging/uninstall-linux.sh

test: $(BUILD)/libstrata_display.so
	python3 -m py_compile native/simulator.py
	python3 tests/test_renderer.py
	python3 tests/test_faceplate.py

clean:
	rm -f $(BUILD)/libstrata_display.so
