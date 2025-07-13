#include "../src/common/sm3_common.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

/* 声明各版本的函数 */
extern void sm3_basic_hash(const uint8_t *data, size_t len, uint8_t *digest);
extern void sm3_basic_batch_hash(const uint8_t *data, size_t len, uint8_t *digest, int iterations);
extern int sm3_basic_test(void);

#ifdef __AVX2__
extern void sm3_simd_hash(const uint8_t *data, size_t len, uint8_t *digest);
extern int sm3_simd_test(void);
#endif

extern void sm3_optimized_hash(const uint8_t *data, size_t len, uint8_t *digest);
extern void sm3_optimized_batch_hash(const uint8_t *data, size_t len, uint8_t *digest, int iterations);
extern int sm3_optimized_test(void);

/**
 * 测试向量结构体
 */
typedef struct {
    const char *name;
    const char *input;
    const uint8_t expected[SM3_DIGEST_SIZE];
} test_vector_t;

/**
 * SM3标准测试向量
 */
static const test_vector_t test_vectors[] = {
    {
        "空字符串",
        "",
        {
            0x1a, 0xb2, 0x1d, 0x83, 0x55, 0xcf, 0xa1, 0x7f,
            0x8e, 0x61, 0x19, 0x48, 0x31, 0xe8, 0x1a, 0x8f,
            0x22, 0xbe, 0xc8, 0xc7, 0x28, 0xfe, 0xfb, 0x74,
            0x7e, 0xd0, 0x35, 0xeb, 0x50, 0x82, 0xaa, 0x2b
        }
    },
    {
        "字符串abc",
        "abc",
        {
            0x66, 0xc7, 0xf0, 0xf4, 0x62, 0xee, 0xed, 0xd9,
            0xd1, 0xf2, 0xd4, 0x6b, 0xdc, 0x10, 0xe4, 0xe2,
            0x41, 0x67, 0xc4, 0x87, 0x5c, 0xf2, 0xf7, 0xa2,
            0x29, 0x7d, 0xa0, 0x2b, 0x8f, 0x4b, 0xa8, 0xe0
        }
    },
    {
        "长字符串",
        "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789",
        {
            0xb9, 0x59, 0x08, 0xf0, 0x92, 0x39, 0x6d, 0x4e,
            0xa3, 0x36, 0x8c, 0x7a, 0x3d, 0x9b, 0x2b, 0x1a,
            0x3c, 0x7f, 0x44, 0x2e, 0xd2, 0x3a, 0x85, 0xc9,
            0x8a, 0x33, 0xb7, 0x8f, 0x0c, 0x5e, 0x2d, 0x1b
        }
    }
};

#define NUM_TEST_VECTORS (sizeof(test_vectors) / sizeof(test_vectors[0]))

/**
 * 打印哈希值
 */
static void print_hash(const uint8_t *hash, const char *label) {
    printf("%s: ", label);
    for (int i = 0; i < SM3_DIGEST_SIZE; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
}

/**
 * 基础功能测试
 */
static int test_functionality() {
    printf("=== SM3功能正确性测试 ===\n\n");
    
    uint8_t digest[SM3_DIGEST_SIZE];
    int passed = 0;
    int total = 0;
    
    for (size_t i = 0; i < NUM_TEST_VECTORS; i++) {
        const test_vector_t *tv = &test_vectors[i];
        
        printf("测试 %zu: %s\n", i + 1, tv->name);
        printf("输入: \"%s\" (长度: %zu)\n", tv->input, strlen(tv->input));
        
        /* 测试基础实现 */
        sm3_basic_hash((const uint8_t *)tv->input, strlen(tv->input), digest);
        print_hash(digest, "基础实现");
        print_hash(tv->expected, "期望结果");
        
        if (memcmp(digest, tv->expected, SM3_DIGEST_SIZE) == 0) {
            printf("✅ 基础实现: 通过\n");
            passed++;
        } else {
            printf("❌ 基础实现: 失败\n");
        }
        total++;
        
        /* 测试优化实现 */
        sm3_optimized_hash((const uint8_t *)tv->input, strlen(tv->input), digest);
        if (memcmp(digest, tv->expected, SM3_DIGEST_SIZE) == 0) {
            printf("✅ 优化实现: 通过\n");
            passed++;
        } else {
            printf("❌ 优化实现: 失败\n");
        }
        total++;
        
#ifdef __AVX2__
        /* 测试SIMD实现 */
        sm3_simd_hash((const uint8_t *)tv->input, strlen(tv->input), digest);
        if (memcmp(digest, tv->expected, SM3_DIGEST_SIZE) == 0) {
            printf("✅ SIMD实现: 通过\n");
            passed++;
        } else {
            printf("❌ SIMD实现: 失败\n");
        }
        total++;
#endif
        
        printf("\n");
    }
    
    printf("功能测试结果: %d/%d 通过\n\n", passed, total);
    return (passed == total);
}

/**
 * 性能基准测试
 */
static void benchmark_performance() {
    printf("=== SM3性能基准测试 ===\n\n");
    
    const size_t test_sizes[] = {64, 256, 1024, 4096, 16384};
    const int iterations = 10000;
    
    for (size_t i = 0; i < sizeof(test_sizes) / sizeof(test_sizes[0]); i++) {
        size_t size = test_sizes[i];
        uint8_t *test_data = malloc(size);
        uint8_t digest[SM3_DIGEST_SIZE];
        
        /* 生成测试数据 */
        for (size_t j = 0; j < size; j++) {
            test_data[j] = (uint8_t)(j & 0xFF);
        }
        
        printf("测试数据大小: %zu 字节, 迭代次数: %d\n", size, iterations);
        
        /* 基础实现基准 */
        clock_t start = clock();
        sm3_basic_batch_hash(test_data, size, digest, iterations);
        clock_t end = clock();
        double basic_time = ((double)(end - start)) / CLOCKS_PER_SEC;
        double basic_throughput = (size * iterations) / (basic_time * 1024 * 1024);
        
        printf("  基础实现: %.3f 秒, %.2f MB/s\n", basic_time, basic_throughput);
        
        /* 优化实现基准 */
        start = clock();
        sm3_optimized_batch_hash(test_data, size, digest, iterations);
        end = clock();
        double optimized_time = ((double)(end - start)) / CLOCKS_PER_SEC;
        double optimized_throughput = (size * iterations) / (optimized_time * 1024 * 1024);
        
        printf("  优化实现: %.3f 秒, %.2f MB/s\n", optimized_time, optimized_throughput);
        
        double speedup = basic_time / optimized_time;
        printf("  加速比: %.2fx\n", speedup);
        
        printf("\n");
        free(test_data);
    }
}

/**
 * 边界条件测试
 */
static void test_edge_cases() {
    printf("=== SM3边界条件测试 ===\n\n");
    
    uint8_t digest[SM3_DIGEST_SIZE];
    sm3_ctx_t ctx;
    
    /* 测试1: 多次更新空数据 */
    printf("测试1: 多次更新空数据\n");
    sm3_init(&ctx);
    for (int i = 0; i < 10; i++) {
        sm3_update(&ctx, NULL, 0);
    }
    sm3_final(&ctx, digest);
    print_hash(digest, "结果");
    print_hash(test_vectors[0].expected, "期望");
    printf("状态: %s\n\n", 
           memcmp(digest, test_vectors[0].expected, SM3_DIGEST_SIZE) == 0 ? "✅通过" : "❌失败");
    
    /* 测试2: 单字节更新 */
    printf("测试2: 单字节更新 \"abc\"\n");
    sm3_init(&ctx);
    sm3_update(&ctx, (const uint8_t *)"a", 1);
    sm3_update(&ctx, (const uint8_t *)"b", 1);
    sm3_update(&ctx, (const uint8_t *)"c", 1);
    sm3_final(&ctx, digest);
    print_hash(digest, "结果");
    print_hash(test_vectors[1].expected, "期望");
    printf("状态: %s\n\n", 
           memcmp(digest, test_vectors[1].expected, SM3_DIGEST_SIZE) == 0 ? "✅通过" : "❌失败");
    
    /* 测试3: 跨块边界 */
    printf("测试3: 跨块边界更新\n");
    uint8_t large_data[128];
    memset(large_data, 0x5A, sizeof(large_data));
    
    /* 方法1: 一次性计算 */
    sm3_hash(large_data, sizeof(large_data), digest);
    uint8_t digest1[SM3_DIGEST_SIZE];
    memcpy(digest1, digest, SM3_DIGEST_SIZE);
    
    /* 方法2: 分块计算 */
    sm3_init(&ctx);
    sm3_update(&ctx, large_data, 60);
    sm3_update(&ctx, large_data + 60, 68);
    sm3_final(&ctx, digest);
    
    print_hash(digest1, "一次性");
    print_hash(digest, "分块计算");
    printf("状态: %s\n\n", 
           memcmp(digest1, digest, SM3_DIGEST_SIZE) == 0 ? "✅通过" : "❌失败");
}

/**
 * 主测试函数
 */
int main() {
    printf("========================================\n");
    printf("        SM3哈希算法优化测试套件\n");
    printf("========================================\n\n");
    
    printf("编译信息:\n");
    printf("  编译器: GCC %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
    printf("  编译时间: %s %s\n", __DATE__, __TIME__);
    printf("  目标架构: %s\n", 
#ifdef __x86_64__
           "x86_64"
#elif defined(__i386__)
           "i386"
#elif defined(__aarch64__)
           "aarch64"
#elif defined(__arm__)
           "arm"
#else
           "unknown"
#endif
    );
    
#ifdef __AVX2__
    printf("  SIMD支持: AVX2\n");
#else
    printf("  SIMD支持: 无\n");
#endif
    
    printf("  优化级别: ");
#ifdef __OPTIMIZE__
    printf("开启\n");
#else
    printf("关闭\n");
#endif
    printf("\n");
    
    /* 运行各项测试 */
    int functionality_ok = test_functionality();
    
    test_edge_cases();
    
    if (functionality_ok) {
        benchmark_performance();
    } else {
        printf("⚠️  功能测试失败，跳过性能测试\n");
    }
    
    /* 运行各版本的内置测试 */
    printf("=== 各版本内置测试 ===\n\n");
    
    printf("基础版本测试:\n");
    int basic_ok = sm3_basic_test();
    printf("\n");
    
#ifdef __AVX2__
    printf("SIMD版本测试:\n");
    int simd_ok = sm3_simd_test();
    printf("\n");
#else
    int simd_ok = 1;  /* 跳过 */
#endif
    
    printf("优化版本测试:\n");
    int optimized_ok = sm3_optimized_test();
    printf("\n");
    
    /* 总结 */
    printf("========================================\n");
    printf("                测试总结\n");
    printf("========================================\n");
    printf("功能正确性: %s\n", functionality_ok ? "✅ 通过" : "❌ 失败");
    printf("基础实现: %s\n", basic_ok ? "✅ 通过" : "❌ 失败");
    printf("SIMD实现: %s\n", simd_ok ? "✅ 通过" : "❌ 失败/跳过");
    printf("优化实现: %s\n", optimized_ok ? "✅ 通过" : "❌ 失败");
    
    int overall_result = functionality_ok && basic_ok && simd_ok && optimized_ok;
    printf("\n总体结果: %s\n", overall_result ? "🎉 全部通过" : "💥 存在失败");
    
    return overall_result ? 0 : 1;
}
