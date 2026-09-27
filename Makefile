.DEFAULT_GOAL := all

CC = cc
STD = -std=gnu11
WARN = -Wall -Wextra -Werror -Wshadow -pedantic
INC = -Iinclude/tree-sitter/lib/include -Iinclude/tree-sitter/lib/src
LDLIBS = -ldl
BUILD_DIR = build
RDIR = $(BUILD_DIR)/release
DDIR = $(BUILD_DIR)/debug
ADIR = $(BUILD_DIR)/asan

BASEFLAGS = $(STD) $(WARN) $(INC)
RCFLAGS = $(BASEFLAGS) -O2 -DNDEBUG
DCFLAGS = $(BASEFLAGS) -O0 -g3
ACFLAGS = $(BASEFLAGS) -O1 -g3 -fsanitize=address,undefined

SRCS = $(wildcard src/*.c)
HDRS = $(wildcard src/*.h)
TS_SRC = include/tree-sitter/lib/src/lib.c

R_OBJS = $(patsubst src/%.c,$(RDIR)/%.o,$(SRCS))
D_OBJS = $(patsubst src/%.c,$(DDIR)/%.o,$(SRCS))
A_OBJS = $(patsubst src/%.c,$(ADIR)/%.o,$(SRCS))

$(BUILD_DIR)/tsag: $(R_OBJS) $(RDIR)/lib.o | $(BUILD_DIR)
	$(CC) $(RCFLAGS) -o $@ $(R_OBJS) $(RDIR)/lib.o $(LDLIBS)

$(BUILD_DIR)/tsag-debug: $(D_OBJS) $(DDIR)/lib.o | $(BUILD_DIR)
	$(CC) $(DCFLAGS) -o $@ $(D_OBJS) $(DDIR)/lib.o $(LDLIBS)

$(BUILD_DIR)/tsag-asan: $(A_OBJS) $(ADIR)/lib.o | $(BUILD_DIR)
	$(CC) $(ACFLAGS) -o $@ $(A_OBJS) $(ADIR)/lib.o $(LDLIBS)

$(BUILD_DIR)/tsdump: $(RDIR)/tsdump.o $(RDIR)/lang.o $(RDIR)/lib.o | $(BUILD_DIR)
	$(CC) $(RCFLAGS) -o $@ $(RDIR)/tsdump.o $(RDIR)/lang.o $(RDIR)/lib.o $(LDLIBS)

$(RDIR)/%.o: src/%.c $(HDRS) | $(RDIR)
	$(CC) $(RCFLAGS) -c $< -o $@

$(DDIR)/%.o: src/%.c $(HDRS) | $(DDIR)
	$(CC) $(DCFLAGS) -c $< -o $@

$(ADIR)/%.o: src/%.c $(HDRS) | $(ADIR)
	$(CC) $(ACFLAGS) -c $< -o $@

$(RDIR)/lib.o: $(TS_SRC) | $(RDIR)
	$(CC) $(RCFLAGS) -c $< -o $@

$(DDIR)/lib.o: $(TS_SRC) | $(DDIR)
	$(CC) $(DCFLAGS) -c $< -o $@

$(ADIR)/lib.o: $(TS_SRC) | $(ADIR)
	$(CC) $(ACFLAGS) -c $< -o $@

$(RDIR)/tsdump.o: tools/tsdump.c $(HDRS) | $(RDIR)
	$(CC) $(RCFLAGS) -Isrc -c $< -o $@

$(BUILD_DIR) $(RDIR) $(DDIR) $(ADIR):
	mkdir -p $@

.PHONY: all debug asan clean run
all: $(BUILD_DIR)/tsag $(BUILD_DIR)/tsdump
debug: $(BUILD_DIR)/tsag-debug
asan: $(BUILD_DIR)/tsag-asan
run: all
	$(BUILD_DIR)/tsag $(ARGS)

clean:
	rm -rf $(BUILD_DIR)
