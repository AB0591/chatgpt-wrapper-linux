CC ?= cc
APP := chatgpt-wrapper
BUILD_DIR := build
SRC := src/main.c src/hotkey.c src/downloads.c src/settings.c src/continuation.c
OBJ := $(SRC:src/%.c=$(BUILD_DIR)/%.o)
DEP := $(OBJ:.o=.d)

CFLAGS += -Wall -Wextra -std=c11 -MMD -MP $(shell pkg-config --cflags gtk4 webkitgtk-6.0)
LDFLAGS += $(shell pkg-config --libs gtk4 webkitgtk-6.0) -lX11

all: $(BUILD_DIR)/$(APP)

$(BUILD_DIR)/$(APP): $(OBJ)
	mkdir -p $(BUILD_DIR)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: src/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BUILD_DIR)/$(APP)
	./$(BUILD_DIR)/$(APP)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean

-include $(DEP)
