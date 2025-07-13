#include "../src/common/sm3_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#define GET_TIME() GetTickCount64()
#define TIME_DIFF(start, end) ((end) - (start))
#else
#include <sys/time.h>
static uint64_t GET_TIME() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000ULL + tv.tv_usec / 1000ULL;
}
#define TIME_DIFF(start, end) ((end) - (start))
#endif

/* 声明各版本函数 */
extern void sm3_basic_hash(const uint8_t *data, size_t len, uint8_t *digest);
extern void sm3_basic_batch_hash(const uint8_t *data, size_t len, uint8_t *digest, int iterations);

#ifdef __AVX2__
extern void sm3_simd_hash(const uint8_t *data, size_t len, uint8_t *digest);
#endif

extern void sm3_optimized_hash(const uint8_t *data, size_t len, uint8_t *digest);
extern void sm3_optimized_batch_hash(const uint8_t *data, size_t len, uint8_t *digest, int iterations);

/**
 * 基准测试配置
 */
typedef struct {
    const char *name;
    size_t data_size;
    int iterations;
    int warmup_iterations;
} benchmark_config_t;

/**
 * 基准测试结果
 */
typedef struct {
    double time_ms;
    double throughput_mbps;
    double cycles_per_byte;
} benchmark_result_t;

/**
 * 预定义的基准测试配置
 */
static const benchmark_config_t benchmark_configs[] = {
    {"小数据块 (64B)", 64, 100000, 1000},
    {"中等数据块 (1KB)", 1024, 10000, 100},
    {"大数据块 (4KB)", 4096, 2500, 25},
    {"超大数据块 (16KB)", 16384, 625, 10},
    {"巨大数据块 (64KB)", 65536, 156, 5},
    {"网络包大小 (1500B)", 1500, 6666, 100},
    {"页面大小 (4096B)", 4096, 2500, 25}
};

#define NUM_CONFIGS (sizeof(benchmark_configs) / sizeof(benchmark_configs[0]))

/**
 * 生成测试数据
 */
static void generate_test_data(uint8_t *data, size_t size, uint32_t seed) {
    srand(seed);
    for (size_t i = 0; i < size; i++) {
        data[i] = (uint8_t)(rand() & 0xFF);
    }
}

/**
 * 执行基准测试
 */
static benchmark_result_t run_benchmark(
    void (*hash_func)(const uint8_t *, size_t, uint8_t *),
    const uint8_t *data,
    size_t data_size,
    int iterations,
    int warmup_iterations
) {
    uint8_t digest[SM3_DIGEST_SIZE];
    benchmark_result_t result = {0};
    
    /* 预热 */
    for (int i = 0; i < warmup_iterations; i++) {
        hash_func(data, data_size, digest);
    }
    
    /* 实际测试 */
    uint64_t start_time = GET_TIME();
    
    for (int i = 0; i < iterations; i++) {
        hash_func(data, data_size, digest);
    }
    
    uint64_t end_time = GET_TIME();
    
    /* 计算结果 */
    result.time_ms = (double)TIME_DIFF(start_time, end_time);
    
    if (result.time_ms > 0) {
        double total_bytes = (double)data_size * iterations;
        result.throughput_mbps = (total_bytes / 1024.0 / 1024.0) / (result.time_ms / 1000.0);
        
        /* 估算每字节周期数（假设3GHz CPU） */
        double cpu_freq_ghz = 3.0;
        double total_cycles = (result.time_ms / 1000.0) * cpu_freq_ghz * 1e9;
        result.cycles_per_byte = total_cycles / total_bytes;
    }
    
    return result;
}

/**
 * 批量基准测试
 */
static benchmark_result_t run_batch_benchmark(
    void (*batch_func)(const uint8_t *, size_t, uint8_t *, int),
    const uint8_t *data,
    size_t data_size,
    int iterations,
    int warmup_iterations
) {
    uint8_t digest[SM3_DIGEST_SIZE];
    benchmark_result_t result = {0};
    
    /* 预热 */
    if (warmup_iterations > 0) {
        batch_func(data, data_size, digest, warmup_iterations);
    }
    
    /* 实际测试 */
    uint64_t start_time = GET_TIME();
    batch_func(data, data_size, digest, iterations);
    uint64_t end_time = GET_TIME();
    
    /* 计算结果 */
    result.time_ms = (double)TIME_DIFF(start_time, end_time);
    
    if (result.time_ms > 0) {
        double total_bytes = (double)data_size * iterations;
        result.throughput_mbps = (total_bytes / 1024.0 / 1024.0) / (result.time_ms / 1000.0);
        
        double cpu_freq_ghz = 3.0;
        double total_cycles = (result.time_ms / 1000.0) * cpu_freq_ghz * 1e9;
        result.cycles_per_byte = total_cycles / total_bytes;
    }
    
    return result;
}

/**
 * 打印基准测试结果
 */
static void print_benchmark_results(const char *version, const benchmark_result_t *results, int count) {
    printf("\n=== %s 性能结果 ===\n", version);
    printf("%-20s %10s %12s %15s\n", "数据大小", "时间(ms)", "吞吐量(MB/s)", "周期/字节");
    printf("%-20s %10s %12s %15s\n", "--------", "-------", "-----------", "--------");
    
    double total_throughput = 0;
    for (int i = 0; i < count; i++) {
        printf("%-20s %10.2f %12.2f %15.2f\n",
               benchmark_configs[i].name,
               results[i].time_ms,
               results[i].throughput_mbps,
               results[i].cycles_per_byte);
        total_throughput += results[i].throughput_mbps;
    }
    
    printf("%-20s %10s %12.2f %15s\n", "平均", "-", total_throughput / count, "-");
}

/**
 * 比较性能结果
 */
static void compare_results(const benchmark_result_t *basic, const benchmark_result_t *optimized, int count) {
    printf("\n=== 性能对比分析 ===\n");
    printf("%-20s %12s %12s %10s\n", "数据大小", "基础版本", "优化版本", "加速比");
    printf("%-20s %12s %12s %10s\n", "--------", "--------", "--------", "------");
    
    double total_speedup = 0;
    for (int i = 0; i < count; i++) {
        double speedup = optimized[i].throughput_mbps / basic[i].throughput_mbps;
        printf("%-20s %12.2f %12.2f %10.2fx\n",
               benchmark_configs[i].name,
               basic[i].throughput_mbps,
               optimized[i].throughput_mbps,
               speedup);
        total_speedup += speedup;
    }
    
    printf("%-20s %12s %12s %10.2fx\n", "平均", "-", "-", total_speedup / count);
}

/**
 * 内存效率测试
 */
static void test_memory_efficiency() {
    printf("\n=== 内存使用效率测试 ===\n");
    
    const size_t test_size = 1024 * 1024;  /* 1MB */
    uint8_t *test_data = malloc(test_size);
    uint8_t digest[SM3_DIGEST_SIZE];
    
    if (!test_data) {
        printf("内存分配失败\n");
        return;
    }
    
    generate_test_data(test_data, test_size, 12345);
    
    printf("测试数据大小: %zu MB\n", test_size / 1024 / 1024);
    printf("SM3上下文大小: %zu 字节\n", sizeof(sm3_ctx_t));
    
    /* 测试不同块大小的处理效率 */
    const size_t block_sizes[] = {64, 256, 1024, 4096, 16384};
    const int num_blocks = sizeof(block_sizes) / sizeof(block_sizes[0]);
    
    printf("\n块大小处理效率:\n");
    printf("%-10s %15s %15s\n", "块大小", "基础版本(MB/s)", "优化版本(MB/s)");
    printf("%-10s %15s %15s\n", "------", "-------------", "-------------");
    
    for (int i = 0; i < num_blocks; i++) {
        size_t block_size = block_sizes[i];
        int iterations = test_size / block_size;
        
        /* 基础版本 */
        uint64_t start = GET_TIME();
        for (int j = 0; j < iterations; j++) {
            sm3_basic_hash(test_data + j * block_size, block_size, digest);
        }
        uint64_t end = GET_TIME();
        double basic_time = TIME_DIFF(start, end) / 1000.0;
        double basic_throughput = (test_size / 1024.0 / 1024.0) / basic_time;
        
        /* 优化版本 */
        start = GET_TIME();
        for (int j = 0; j < iterations; j++) {
            sm3_optimized_hash(test_data + j * block_size, block_size, digest);
        }
        end = GET_TIME();
        double opt_time = TIME_DIFF(start, end) / 1000.0;
        double opt_throughput = (test_size / 1024.0 / 1024.0) / opt_time;
        
        printf("%-10zu %15.2f %15.2f\n", block_size, basic_throughput, opt_throughput);
    }
    
    free(test_data);
}

/**
 * CPU缓存效率测试
 */
static void test_cache_efficiency() {
    printf("\n=== CPU缓存效率测试 ===\n");
    
    /* 测试不同大小数据对缓存的影响 */
    const size_t cache_sizes[] = {
        8 * 1024,      /* L1 Cache 8KB */
        256 * 1024,    /* L2 Cache 256KB */
        8 * 1024 * 1024, /* L3 Cache 8MB */
        64 * 1024 * 1024 /* 超出缓存 64MB */
    };
    const char *cache_names[] = {"L1范围", "L2范围", "L3范围", "超出缓存"};
    const int num_sizes = sizeof(cache_sizes) / sizeof(cache_sizes[0]);
    
    printf("%-10s %15s %15s %10s\n", "缓存级别", "基础版本(MB/s)", "优化版本(MB/s)", "加速比");
    printf("%-10s %15s %15s %10s\n", "--------", "-------------", "-------------", "------");
    
    for (int i = 0; i < num_sizes; i++) {
        size_t data_size = cache_sizes[i];
        uint8_t *test_data = malloc(data_size);
        uint8_t digest[SM3_DIGEST_SIZE];
        
        if (!test_data) continue;
        
        generate_test_data(test_data, data_size, 54321);
        
        int iterations = (1024 * 1024) / (data_size / 1024) + 1;  /* 至少处理1GB */
        
        /* 基础版本 */
        uint64_t start = GET_TIME();
        for (int j = 0; j < iterations; j++) {
            sm3_basic_hash(test_data, data_size, digest);
        }
        uint64_t end = GET_TIME();
        double basic_time = TIME_DIFF(start, end) / 1000.0;
        double basic_throughput = (data_size * iterations / 1024.0 / 1024.0) / basic_time;
        
        /* 优化版本 */
        start = GET_TIME();
        for (int j = 0; j < iterations; j++) {
            sm3_optimized_hash(test_data, data_size, digest);
        }
        end = GET_TIME();
        double opt_time = TIME_DIFF(start, end) / 1000.0;
        double opt_throughput = (data_size * iterations / 1024.0 / 1024.0) / opt_time;
        
        double speedup = opt_throughput / basic_throughput;
        printf("%-10s %15.2f %15.2f %10.2fx\n", 
               cache_names[i], basic_throughput, opt_throughput, speedup);
        
        free(test_data);
    }
}

/**
 * 主函数
 */
int main() {
    printf("========================================\n");
    printf("      SM3哈希算法性能基准测试\n");
    printf("========================================\n");
    
    printf("系统信息:\n");
    printf("  指针大小: %zu 位\n", sizeof(void*) * 8);
    printf("  时间戳精度: 毫秒\n");
    
#ifdef __AVX2__
    printf("  SIMD支持: AVX2\n");
#else
    printf("  SIMD支持: 无\n");
#endif
    
    printf("  编译优化: ");
#ifdef __OPTIMIZE__
    printf("开启\n");
#else
    printf("关闭\n");
#endif
    printf("\n");
    
    /* 分配测试数据 */
    const size_t max_data_size = 65536;
    uint8_t *test_data = malloc(max_data_size);
    if (!test_data) {
        printf("内存分配失败\n");
        return 1;
    }
    
    generate_test_data(test_data, max_data_size, 42);
    
    /* 运行基准测试 */
    benchmark_result_t basic_results[NUM_CONFIGS];
    benchmark_result_t optimized_results[NUM_CONFIGS];
    
    printf("正在运行基准测试...\n");
    
    for (int i = 0; i < NUM_CONFIGS; i++) {
        const benchmark_config_t *config = &benchmark_configs[i];
        
        printf("  测试 %d/%d: %s\n", i + 1, NUM_CONFIGS, config->name);
        
        /* 基础版本 */
        basic_results[i] = run_batch_benchmark(
            sm3_basic_batch_hash,
            test_data,
            config->data_size,
            config->iterations,
            config->warmup_iterations
        );
        
        /* 优化版本 */
        optimized_results[i] = run_batch_benchmark(
            sm3_optimized_batch_hash,
            test_data,
            config->data_size,
            config->iterations,
            config->warmup_iterations
        );
    }
    
    /* 打印结果 */
    print_benchmark_results("基础版本", basic_results, NUM_CONFIGS);
    print_benchmark_results("优化版本", optimized_results, NUM_CONFIGS);
    compare_results(basic_results, optimized_results, NUM_CONFIGS);
    
    /* 运行额外测试 */
    test_memory_efficiency();
    test_cache_efficiency();
    
    /* 生成性能报告 */
    printf("\n=== 性能总结 ===\n");
    double avg_basic = 0, avg_optimized = 0;
    for (int i = 0; i < NUM_CONFIGS; i++) {
        avg_basic += basic_results[i].throughput_mbps;
        avg_optimized += optimized_results[i].throughput_mbps;
    }
    avg_basic /= NUM_CONFIGS;
    avg_optimized /= NUM_CONFIGS;
    
    printf("平均吞吐量:\n");
    printf("  基础实现: %.2f MB/s\n", avg_basic);
    printf("  优化实现: %.2f MB/s\n", avg_optimized);
    printf("  总体加速比: %.2fx\n", avg_optimized / avg_basic);
    
    if (avg_optimized / avg_basic >= 2.0) {
        printf("🎉 优化效果显著！\n");
    } else if (avg_optimized / avg_basic >= 1.5) {
        printf("✅ 优化效果良好\n");
    } else {
        printf("⚠️  优化效果有限\n");
    }
    
    free(test_data);
    return 0;
}
