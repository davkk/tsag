CC = cc
STD = -std=gnu11 # TODO: maybe I should only use -std=gnu11
WARN = -Wall -Wextra -Werror -Wshadow -pedantic
INC = -Iinclude/tree-sitter/lib/include -Iinclude/tree-sitter/lib/src
SRCS = $(wildcard src/*.c) include/tree-sitter/lib/src/lib.c
HDRS = $(wildcard src/*.h)
LDLIBS = -ldl
BUILD_DIR = build

CFLAGS += $(STD) $(WARN) $(INC)

$(BUILD_DIR)/tsag: CFLAGS += -O2 -DNDEBUG
$(BUILD_DIR)/tsag-debug: CFLAGS += -O0 -g3
$(BUILD_DIR)/tsag-asan: CFLAGS += -O1 -g3 -fsanitize=address,undefined

$(BUILD_DIR)/tsag $(BUILD_DIR)/tsag-debug $(BUILD_DIR)/tsag-asan: $(SRCS) $(HDRS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

TSDUMP_SRCS = tools/tsdump.c src/lang.c src/queries.h include/tree-sitter/lib/src/lib.c
$(BUILD_DIR)/tsdump: CFLAGS += -Isrc -O1 -g3
$(BUILD_DIR)/tsdump: $(TSDUMP_SRCS) $(HDRS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ tools/tsdump.c src/lang.c include/tree-sitter/lib/src/lib.c $(LDLIBS)

$(BUILD_DIR):
	mkdir -p $@

.PHONY: all debug asan clean run
all: $(BUILD_DIR)/tsag $(BUILD_DIR)/tsdump
debug: $(BUILD_DIR)/tsag-debug
asan: $(BUILD_DIR)/tsag-asan
run: all
	$(BUILD_DIR)/tsag $(ARGS)

clean:
	rm -rf $(BUILD_DIR)
