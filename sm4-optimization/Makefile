# SM4 Optimization Project Makefile

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
SIMD_FLAGS = -mavx2 -mavx512f
DEBUG_FLAGS = -g -DDEBUG

# Directories
SRC_DIR = src
TEST_DIR = tests
BENCH_DIR = benchmarks
BUILD_DIR = build

# Source files
BASIC_SRCS = $(SRC_DIR)/basic/sm4_basic.c $(SRC_DIR)/common/utils.c
TTABLE_SRCS = $(SRC_DIR)/ttable/sm4_ttable.c $(SRC_DIR)/common/utils.c
SIMD_SRCS = $(SRC_DIR)/simd/sm4_simd.c $(SRC_DIR)/common/utils.c

# Test files
TEST_SRCS = $(TEST_DIR)/test_sm4.c

# Benchmark files
BENCH_SRCS = $(BENCH_DIR)/benchmark.c

# Object files
BASIC_OBJS = $(BASIC_SRCS:%.c=$(BUILD_DIR)/%.o)
TTABLE_OBJS = $(TTABLE_SRCS:%.c=$(BUILD_DIR)/%.o)
SIMD_OBJS = $(SIMD_SRCS:%.c=$(BUILD_DIR)/%.o)
TEST_OBJS = $(TEST_SRCS:%.c=$(BUILD_DIR)/%.o)
BENCH_OBJS = $(BENCH_SRCS:%.c=$(BUILD_DIR)/%.o)

# Targets
BASIC_TARGET = $(BUILD_DIR)/sm4_basic
TTABLE_TARGET = $(BUILD_DIR)/sm4_ttable
SIMD_TARGET = $(BUILD_DIR)/sm4_simd
TEST_TARGET = $(BUILD_DIR)/test_sm4
BENCH_TARGET = $(BUILD_DIR)/benchmark

.PHONY: all clean test benchmark basic ttable simd

all: basic ttable simd test benchmark

basic: $(BASIC_TARGET)
ttable: $(TTABLE_TARGET)
simd: $(SIMD_TARGET)
test: $(TEST_TARGET)
benchmark: $(BENCH_TARGET)

# Create build directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/$(SRC_DIR)/basic
	mkdir -p $(BUILD_DIR)/$(SRC_DIR)/ttable
	mkdir -p $(BUILD_DIR)/$(SRC_DIR)/simd
	mkdir -p $(BUILD_DIR)/$(SRC_DIR)/common
	mkdir -p $(BUILD_DIR)/$(TEST_DIR)
	mkdir -p $(BUILD_DIR)/$(BENCH_DIR)

# Basic implementation
$(BASIC_TARGET): $(BASIC_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/$(SRC_DIR)/basic/%.o: $(SRC_DIR)/basic/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# T-table implementation
$(TTABLE_TARGET): $(TTABLE_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/$(SRC_DIR)/ttable/%.o: $(SRC_DIR)/ttable/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# SIMD implementation
$(SIMD_TARGET): $(SIMD_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SIMD_FLAGS) -o $@ $^

$(BUILD_DIR)/$(SRC_DIR)/simd/%.o: $(SRC_DIR)/simd/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SIMD_FLAGS) -c -o $@ $<

# Common objects
$(BUILD_DIR)/$(SRC_DIR)/common/%.o: $(SRC_DIR)/common/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Test target
$(TEST_TARGET): $(TEST_OBJS) $(BUILD_DIR)/$(SRC_DIR)/basic/sm4_basic.o $(BUILD_DIR)/$(SRC_DIR)/common/utils.o | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Benchmark target
$(BENCH_TARGET): $(BENCH_OBJS) $(BASIC_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/$(BENCH_DIR)/%.o: $(BENCH_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Run tests
run-test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Run benchmarks
run-benchmark: $(BENCH_TARGET)
	./$(BENCH_TARGET)

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Debug builds
debug: CFLAGS += $(DEBUG_FLAGS)
debug: all
