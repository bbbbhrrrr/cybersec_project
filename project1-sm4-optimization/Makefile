# SM4优化实现项目Makefile
# 支持基本实现、T-table、SIMD、AES-NI、GFNI、VPROLD优化和GCM模式

CC = gcc
CFLAGS = -Wall -Wextra -O3 -march=native -std=c99
INCLUDES = -Isrc/common -Isrc/basic -Isrc/ttable -Isrc/simd -Isrc/aesni -Isrc/gfni -Isrc/vprold -Isrc/gcm
SRCDIR = src
TESTDIR = tests
BENCHDIR = benchmarks
BUILDDIR = build

# 源文件
COMMON_SRCS = $(SRCDIR)/common/sm4_common.c
BASIC_SRCS = $(SRCDIR)/basic/sm4_basic.c
TTABLE_SRCS = $(SRCDIR)/ttable/sm4_ttable.c
SIMD_SRCS = $(SRCDIR)/simd/sm4_simd.c
AESNI_SRCS = $(SRCDIR)/aesni/sm4_aesni.c
GFNI_SRCS = $(SRCDIR)/gfni/sm4_gfni.c
VPROLD_SRCS = $(SRCDIR)/vprold/sm4_vprold.c
GCM_SRCS = $(SRCDIR)/gcm/sm4_gcm.c

# 测试文件
TEST_SRCS = $(TESTDIR)/test_sm4.c
TEST_OPT_SRCS = $(TESTDIR)/test_sm4_optimization.c
BENCH_SRCS = $(BENCHDIR)/benchmark.c $(BENCHDIR)/benchmark_simd.c

# 目标文件
COMMON_OBJS = $(COMMON_SRCS:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
BASIC_OBJS = $(BASIC_SRCS:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
TTABLE_OBJS = $(TTABLE_SRCS:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
SIMD_OBJS = $(SIMD_SRCS:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
AESNI_OBJS = $(AESNI_SRCS:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
GFNI_OBJS = $(GFNI_SRCS:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
VPROLD_OBJS = $(VPROLD_SRCS:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
GCM_OBJS = $(GCM_SRCS:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

# 所有对象文件
ALL_OBJS = $(COMMON_OBJS) $(BASIC_OBJS) $(TTABLE_OBJS) $(SIMD_OBJS) $(AESNI_OBJS) $(GFNI_OBJS) $(VPROLD_OBJS) $(GCM_OBJS)

# 可执行文件
TARGETS = $(BUILDDIR)/test_sm4 $(BUILDDIR)/test_sm4_ttable $(BUILDDIR)/test_sm4_simd \
          $(BUILDDIR)/test_sm4_optimization $(BUILDDIR)/benchmark $(BUILDDIR)/benchmark_simd

.PHONY: all clean test test-optimization benchmark

all: $(TARGETS)

# 创建构建目录
$(BUILDDIR):
	mkdir -p $(BUILDDIR)/common $(BUILDDIR)/basic $(BUILDDIR)/ttable $(BUILDDIR)/simd \
	         $(BUILDDIR)/aesni $(BUILDDIR)/gfni $(BUILDDIR)/vprold $(BUILDDIR)/gcm

# 编译通用模块
$(BUILDDIR)/common/%.o: $(SRCDIR)/common/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 编译基本实现
$(BUILDDIR)/basic/%.o: $(SRCDIR)/basic/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 编译T-table实现
$(BUILDDIR)/ttable/%.o: $(SRCDIR)/ttable/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 编译SIMD实现
$(BUILDDIR)/simd/%.o: $(SRCDIR)/simd/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -msse4.1 -mavx2 -c $< -o $@

# 编译AES-NI实现
$(BUILDDIR)/aesni/%.o: $(SRCDIR)/aesni/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -maes -mpclmul -msse4.1 -mavx2 -c $< -o $@

# 编译GFNI实现
$(BUILDDIR)/gfni/%.o: $(SRCDIR)/gfni/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -mgfni -mavx512f -mavx512vl -c $< -o $@

# 编译VPROLD实现
$(BUILDDIR)/vprold/%.o: $(SRCDIR)/vprold/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -mavx512f -mavx512vl -c $< -o $@

# 编译GCM实现
$(BUILDDIR)/gcm/%.o: $(SRCDIR)/gcm/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -mpclmul -msse4.1 -mavx2 -c $< -o $@

# 链接测试程序
$(BUILDDIR)/test_sm4: $(TESTDIR)/test_sm4.c $(COMMON_OBJS) $(BASIC_OBJS) | $(BUILDDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

$(BUILDDIR)/test_sm4_ttable: $(TESTDIR)/test_sm4.c $(COMMON_OBJS) $(TTABLE_OBJS) | $(BUILDDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -DSM4_USE_TTABLE -o $@ $^

$(BUILDDIR)/test_sm4_simd: $(TESTDIR)/test_sm4.c $(COMMON_OBJS) $(SIMD_OBJS) | $(BUILDDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -msse4.1 -mavx2 -DSM4_USE_SIMD -o $@ $^

# 综合优化测试程序
$(BUILDDIR)/test_sm4_optimization: $(TEST_OPT_SRCS) $(ALL_OBJS) | $(BUILDDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -maes -mpclmul -mgfni -mavx512f -mavx512vl -msse4.1 -mavx2 -o $@ $^

# 链接性能测试程序
$(BUILDDIR)/benchmark: $(BENCHDIR)/benchmark.c $(COMMON_OBJS) $(BASIC_OBJS) $(TTABLE_OBJS) | $(BUILDDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

$(BUILDDIR)/benchmark_simd: $(BENCHDIR)/benchmark_simd.c $(COMMON_OBJS) $(SIMD_OBJS) | $(BUILDDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -msse4.1 -mavx2 -o $@ $^

# 运行基本测试
test: $(BUILDDIR)/test_sm4 $(BUILDDIR)/test_sm4_ttable $(BUILDDIR)/test_sm4_simd
	@echo "Running basic implementation test..."
	$(BUILDDIR)/test_sm4
	@echo "Running T-table implementation test..."
	$(BUILDDIR)/test_sm4_ttable
	@echo "Running SIMD implementation test..."
	$(BUILDDIR)/test_sm4_simd

# 运行综合优化测试
test-optimization: $(BUILDDIR)/test_sm4_optimization
	@echo "Running comprehensive optimization test..."
	$(BUILDDIR)/test_sm4_optimization

# 运行所有测试
test-all: test test-optimization

# 运行性能测试
benchmark: $(BUILDDIR)/benchmark $(BUILDDIR)/benchmark_simd
	@echo "Running performance benchmarks..."
	$(BUILDDIR)/benchmark
	$(BUILDDIR)/benchmark_simd

# 清理构建文件
clean:
	rm -rf $(BUILDDIR)

# 安装依赖（仅限Ubuntu/Debian）
install-deps:
	sudo apt-get update
	sudo apt-get install -y build-essential gcc-multilib

# 显示优化特性支持情况
info: $(BUILDDIR)/test_sm4_optimization
	@echo "Checking CPU optimization support..."
	$(BUILDDIR)/test_sm4_optimization | grep -E "(SIMD|AES-NI|GFNI|VPROLD) (Info|not available)"

# 显示帮助信息
help:
	@echo "Available targets:"
	@echo "  all               - Build all targets"
	@echo "  test              - Run basic tests"
	@echo "  test-optimization - Run comprehensive optimization tests"
	@echo "  test-all          - Run all tests"
	@echo "  benchmark         - Run performance benchmarks"
	@echo "  info              - Show CPU optimization support"
	@echo "  clean             - Clean build files"
	@echo "  help              - Show this help message"
