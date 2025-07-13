# SM3 哈希算法优化实现 Makefile
# 支持多种优化版本的编译和测试

# 编译器设置
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
SIMD_FLAGS = -mavx2 -msse4.2
OPT_FLAGS = -O3 -funroll-loops -finline-functions

# 目录设置
SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests
BENCH_DIR = benchmarks

# 源文件定义
COMMON_SRCS = $(SRC_DIR)/common/sm3_common.c $(SRC_DIR)/common/utils.c
BASIC_SRCS = $(SRC_DIR)/basic/sm3_basic.c
SIMD_SRCS = $(SRC_DIR)/simd/sm3_simd.c
OPT_SRCS = $(SRC_DIR)/optimized/sm3_optimized.c

# 测试文件
TEST_SRCS = $(TEST_DIR)/test_sm3.c
BENCH_SRCS = $(BENCH_DIR)/benchmark_sm3.c

# 目标文件
BASIC_OBJ = $(BUILD_DIR)/sm3_basic.o $(BUILD_DIR)/sm3_common.o $(BUILD_DIR)/utils.o
SIMD_OBJ = $(BUILD_DIR)/sm3_simd.o $(BUILD_DIR)/sm3_common.o $(BUILD_DIR)/utils.o
OPT_OBJ = $(BUILD_DIR)/sm3_optimized.o $(BUILD_DIR)/sm3_common.o $(BUILD_DIR)/utils.o

# 可执行文件
BASIC_TEST = $(BUILD_DIR)/test_sm3_basic
SIMD_TEST = $(BUILD_DIR)/test_sm3_simd
OPT_TEST = $(BUILD_DIR)/test_sm3_optimized
BENCHMARK = $(BUILD_DIR)/benchmark_sm3

# 默认目标
all: $(BASIC_TEST) $(SIMD_TEST) $(OPT_TEST) $(BENCHMARK)

# 创建build目录
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# 基础版本编译
$(BUILD_DIR)/sm3_basic.o: $(SRC_DIR)/basic/sm3_basic.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -I$(SRC_DIR)

$(BUILD_DIR)/sm3_common.o: $(SRC_DIR)/common/sm3_common.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -I$(SRC_DIR)

$(BUILD_DIR)/utils.o: $(SRC_DIR)/common/utils.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -I$(SRC_DIR)

# SIMD版本编译
$(BUILD_DIR)/sm3_simd.o: $(SRC_DIR)/simd/sm3_simd.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SIMD_FLAGS) -c $< -o $@ -I$(SRC_DIR)

# 优化版本编译
$(BUILD_DIR)/sm3_optimized.o: $(SRC_DIR)/optimized/sm3_optimized.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OPT_FLAGS) -c $< -o $@ -I$(SRC_DIR)

# 测试程序编译
$(BASIC_TEST): $(BASIC_OBJ) $(TEST_DIR)/test_sm3.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_sm3.c $(BASIC_OBJ) -o $@ -I$(SRC_DIR) -DUSE_BASIC

$(SIMD_TEST): $(SIMD_OBJ) $(TEST_DIR)/test_sm3.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SIMD_FLAGS) $(TEST_DIR)/test_sm3.c $(SIMD_OBJ) -o $@ -I$(SRC_DIR) -DUSE_SIMD

$(OPT_TEST): $(OPT_OBJ) $(TEST_DIR)/test_sm3.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OPT_FLAGS) $(TEST_DIR)/test_sm3.c $(OPT_OBJ) -o $@ -I$(SRC_DIR) -DUSE_OPTIMIZED

# 基准测试程序
$(BENCHMARK): $(BASIC_OBJ) $(SIMD_OBJ) $(OPT_OBJ) $(BENCH_DIR)/benchmark_sm3.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SIMD_FLAGS) $(BENCH_DIR)/benchmark_sm3.c $(BASIC_OBJ) $(SIMD_OBJ) $(OPT_OBJ) -o $@ -I$(SRC_DIR)

# 清理编译文件
clean:
	rm -rf $(BUILD_DIR)/*

# 运行所有测试
test: $(BASIC_TEST) $(SIMD_TEST) $(OPT_TEST)
	@echo "运行基础版本测试..."
	./$(BASIC_TEST)
	@echo "运行SIMD版本测试..."
	./$(SIMD_TEST)
	@echo "运行优化版本测试..."
	./$(OPT_TEST)

# 运行性能基准测试
benchmark: $(BENCHMARK)
	@echo "运行性能基准测试..."
	./$(BENCHMARK)

# 安装目标
install: all
	@echo "SM3实现编译完成"

# 帮助信息
help:
	@echo "SM3哈希算法优化实现"
	@echo "可用目标:"
	@echo "  all        - 编译所有版本"
	@echo "  test       - 运行所有测试"
	@echo "  benchmark  - 运行性能测试"
	@echo "  clean      - 清理编译文件"
	@echo "  help       - 显示此帮助信息"

.PHONY: all clean test benchmark install help
