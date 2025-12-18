SHELL := /usr/bin/env bash

SRC_DIR := src
OUT_DIR := result

# Release flags (used only for web now)
CFLAGS := -Os -Wall -I$(SRC_DIR)
LDFLAGS := -lraylib -lm -lpthread -ldl -lrt

# Desktop debug flags
DBGFLAGS := -g -O0 -Wall -I$(SRC_DIR)

WEB_CFLAGS := -Os -Wall -I$(SRC_DIR) -I$(RAYLIB_INCLUDE_PATH) -DPLATFORM_WEB
WEB_LDFLAGS := -L$(RAYLIB_LIB_PATH) -s USE_GLFW=3 -s ASYNCIFY -s MINIFY_HTML=0 \
               --shell-file shell.html --preload-file $(SRC_DIR)/resources@resources \
               -sEXPORTED_FUNCTIONS=['_setStage','_main'] \
               -sEXPORTED_RUNTIME_METHODS=['requestFullscreen','cwrap'] \
               -s TOTAL_STACK=64MB -s INITIAL_MEMORY=128MB -s ASSERTIONS

# Helper macro: runs bear if available
define maybe_bear
	@if command -v bear >/dev/null 2>&1; then \
		bear -- $(1); \
	else \
		$(1); \
	fi
endef

all: desktop web

###
### Desktop
###
desktop: desktop-build desktop-run

desktop-build:
	mkdir -p $(OUT_DIR)
	$(call maybe_bear,cc -o $(OUT_DIR)/game \
		$(SRC_DIR)/main.c \
		$(DBGFLAGS) $(LDFLAGS))
	cp -r $(SRC_DIR)/resources $(OUT_DIR)/

desktop-run:
	cd ${OUT_DIR} && ./game

###
### Web
###
web: web-build web-run

web-build:
	mkdir -p $(OUT_DIR)
	$(call maybe_bear,emcc -o $(OUT_DIR)/index.html \
		$(SRC_DIR)/main.c $(RAYLIB_LIB_PATH)/libraylib.a \
		$(WEB_CFLAGS) $(WEB_LDFLAGS))
	cp -r $(SRC_DIR)/resources $(OUT_DIR)/

web-run:
	cd $(OUT_DIR) && emrun --no-browser index.html

clean:
	rm -rf $(OUT_DIR)/*

.PHONY: all desktop desktop-build desktop-run web web-build web-run clean
