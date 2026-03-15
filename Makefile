CC ?= cc
APP := chatgpt-wrapper
BUILD_DIR := build
SRC := src/main.c src/hotkey.c src/downloads.c src/settings.c

CFLAGS += -Wall -Wextra -std=c11 $(shell pkg-config --cflags gtk4 webkitgtk-6.0)
LDFLAGS += $(shell pkg-config --libs gtk4 webkitgtk-6.0) -lX11

all: $(BUILD_DIR)/$(APP)

$(BUILD_DIR)/$(APP): $(SRC)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

run: $(BUILD_DIR)/$(APP)
	./$(BUILD_DIR)/$(APP)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean
