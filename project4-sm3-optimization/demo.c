#include "../src/common/sm3_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * 简单的Project 4演示程序
 * 测试SM3基础实现的正确性和基本性能
 */

// 测试向量
typedef struct {
    const char *input;
    const char *expected;
} test_vector_t;

static const test_vector_t test_vectors[] = {
    {
        "",
        "1ab21d8355cfa17f8e61194831e81a8f22bec8c728fefb747ed035eb5082aa2b"
    },
    {
        "abc",
        "66c7f0f462eeedd9d1f2d46bdc10e4e24167c4875cf2f7a2297da02b8f4ba8e0"
    },
    {
        "message digest",
        "c522a942e89bd80d97dd666e7a5531b36188c9817149e9b258dfe51ece98ed77"
    },
    {
        "abcdefghijklmnopqrstuvwxyz",
        "b80fe97a4da24afc277564f66a359ef440462ad28dcc6d63adb24d5c20a61595"
    }
};

static const size_t num_test_vectors = sizeof(test_vectors) / sizeof(test_vectors[0]);

/**
 * 十六进制字符串转字节数组
 */
static void hex_to_bytes(const char *hex, uint8_t *bytes) {
    size_t len = strlen(hex);
    unsigned int temp;
    for (size_t i = 0; i < len; i += 2) {
        sscanf(hex + i, "%2x", &temp);
        bytes[i/2] = (uint8_t)temp;
    }
}

/**
 * 字节数组转十六进制字符串
 */
static void bytes_to_hex(const uint8_t *bytes, size_t len, char *hex) {
    for (size_t i = 0; i < len; i++) {
        sprintf(hex + i*2, "%02x", bytes[i]);
    }
    hex[len*2] = '\0';
}

/**
 * 测试SM3基础实现
 */
static int test_basic_sm3(void) {
    printf("=== Testing SM3 Basic Implementation ===\n");
    
    int passed = 0;
    
    for (size_t i = 0; i < num_test_vectors; i++) {
        uint8_t digest[SM3_DIGEST_SIZE];
        uint8_t expected[SM3_DIGEST_SIZE];
        char result_hex[SM3_DIGEST_SIZE * 2 + 1];
        
        // 计算哈希值
        sm3_hash((const uint8_t *)test_vectors[i].input, 
                 strlen(test_vectors[i].input), digest);
        
        // 转换为十六进制字符串
        bytes_to_hex(digest, SM3_DIGEST_SIZE, result_hex);
        
        // 转换期望值
        hex_to_bytes(test_vectors[i].expected, expected);
        
        // 比较结果
        if (memcmp(digest, expected, SM3_DIGEST_SIZE) == 0) {
            printf("  Test %d: PASSED\n", (int)(i + 1));
            printf("    Input: \"%s\"\n", test_vectors[i].input);
            printf("    Hash:  %s\n", result_hex);
            passed++;
        } else {
            printf("  Test %d: FAILED\n", (int)(i + 1));
            printf("    Input:    \"%s\"\n", test_vectors[i].input);
            printf("    Expected: %s\n", test_vectors[i].expected);
            printf("    Got:      %s\n", result_hex);
        }
        printf("\n");
    }
    
    printf("Basic SM3: %d/%d tests passed\n\n", passed, (int)num_test_vectors);
    return passed == (int)num_test_vectors;
}

/**
 * 简单的性能测试
 */
static void performance_test(void) {
    printf("=== SM3 Performance Test ===\n");
    
    const size_t test_sizes[] = {64, 1024, 4096, 16384};
    const size_t num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    const int iterations = 10000;
    
    for (size_t i = 0; i < num_sizes; i++) {
        size_t size = test_sizes[i];
        
        // 生成测试数据
        uint8_t *test_data = malloc(size);
        if (!test_data) continue;
        
        for (size_t j = 0; j < size; j++) {
            test_data[j] = (uint8_t)(j % 256);
        }
        
        printf("Testing with %d bytes (%d iterations):\n", (int)size, iterations);
        
        uint8_t hash[SM3_DIGEST_SIZE];
        
        // 测量时间
        clock_t start = clock();
        
        for (int iter = 0; iter < iterations; iter++) {
            sm3_hash(test_data, size, hash);
        }
        
        clock_t end = clock();
        
        double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
        double total_bytes = (double)size * iterations;
        double throughput = total_bytes / (time_taken * 1024 * 1024); // MB/s
        
        printf("  Time: %.3f seconds\n", time_taken);
        printf("  Throughput: %.2f MB/s\n", throughput);
        printf("  Hash: ");
        for (int k = 0; k < 8; k++) {
            printf("%02x", hash[k]);
        }
        printf("...\n\n");
        
        free(test_data);
    }
}

/**
 * 演示SM3算法的基本特性
 */
static void demonstrate_sm3_properties(void) {
    printf("=== SM3 Algorithm Properties Demo ===\n");
    
    // 1. 雪崩效应演示
    printf("1. Avalanche Effect Demonstration:\n");
    const char *msg1 = "Hello, SM3!";
    const char *msg2 = "Hello, SM4!"; // 只改变一个字符
    
    uint8_t hash1[SM3_DIGEST_SIZE], hash2[SM3_DIGEST_SIZE];
    char hex1[65], hex2[65];
    
    sm3_hash((const uint8_t *)msg1, strlen(msg1), hash1);
    sm3_hash((const uint8_t *)msg2, strlen(msg2), hash2);
    
    bytes_to_hex(hash1, SM3_DIGEST_SIZE, hex1);
    bytes_to_hex(hash2, SM3_DIGEST_SIZE, hex2);
    
    printf("  Message 1: \"%s\"\n", msg1);
    printf("  Hash 1:    %s\n", hex1);
    printf("  Message 2: \"%s\"\n", msg2);
    printf("  Hash 2:    %s\n", hex2);
    
    // 计算不同的位数
    int diff_bits = 0;
    for (int i = 0; i < SM3_DIGEST_SIZE; i++) {
        uint8_t xor_result = hash1[i] ^ hash2[i];
        for (int j = 0; j < 8; j++) {
            if (xor_result & (1 << j)) diff_bits++;
        }
    }
    printf("  Different bits: %d out of %d (%.1f%%)\n\n", 
           diff_bits, SM3_DIGEST_SIZE * 8, 
           (double)diff_bits / (SM3_DIGEST_SIZE * 8) * 100);
    
    // 2. 确定性演示
    printf("2. Deterministic Property:\n");
    uint8_t hash_check[SM3_DIGEST_SIZE];
    char hex_check[65];
    
    sm3_hash((const uint8_t *)msg1, strlen(msg1), hash_check);
    bytes_to_hex(hash_check, SM3_DIGEST_SIZE, hex_check);
    
    printf("  Same input always produces same output:\n");
    printf("  First hash:  %s\n", hex1);
    printf("  Second hash: %s\n", hex_check);
    printf("  Match: %s\n\n", memcmp(hash1, hash_check, SM3_DIGEST_SIZE) == 0 ? "YES" : "NO");
    
    // 3. 长度影响演示
    printf("3. Length Sensitivity:\n");
    const char *short_msg = "test";
    const char *long_msg = "test message with much longer content to demonstrate length sensitivity";
    
    uint8_t short_hash[SM3_DIGEST_SIZE], long_hash[SM3_DIGEST_SIZE];
    char short_hex[65], long_hex[65];
    
    sm3_hash((const uint8_t *)short_msg, strlen(short_msg), short_hash);
    sm3_hash((const uint8_t *)long_msg, strlen(long_msg), long_hash);
    
    bytes_to_hex(short_hash, SM3_DIGEST_SIZE, short_hex);
    bytes_to_hex(long_hash, SM3_DIGEST_SIZE, long_hex);
    
    printf("  Short message (%d bytes): \"%s\"\n", (int)strlen(short_msg), short_msg);
    printf("  Short hash: %s\n", short_hex);
    printf("  Long message (%d bytes): \"%s\"\n", (int)strlen(long_msg), long_msg);
    printf("  Long hash:  %s\n", long_hex);
    printf("\n");
}

/**
 * 主函数
 */
int main(void) {
    printf("===============================================\n");
    printf("        Project 4: SM3 Implementation Demo     \n");
    printf("===============================================\n\n");
    
    // 运行基础测试
    int basic_tests_passed = test_basic_sm3();
    
    // 演示SM3特性
    demonstrate_sm3_properties();
    
    // 性能测试
    performance_test();
    
    // 总结
    printf("=== Project 4 Demo Summary ===\n");
    printf("✓ SM3 Implementation: %s\n", basic_tests_passed ? "WORKING" : "FAILED");
    printf("✓ Standard Test Vectors: %s\n", basic_tests_passed ? "PASSED" : "FAILED");
    printf("✓ Algorithm Properties: DEMONSTRATED\n");
    printf("✓ Performance Analysis: COMPLETED\n");
    printf("\nProject 4 Status: %s\n", basic_tests_passed ? "SUCCESS" : "FAILED");
    
    printf("\n=== Next Steps ===\n");
    printf("1. Implement SIMD optimizations (AVX2)\n");
    printf("2. Add advanced optimization techniques\n");
    printf("3. Implement length-extension attack demo\n");
    printf("4. Build RFC6962 Merkle tree implementation\n");
    printf("5. Comprehensive performance benchmarking\n");
    
    return basic_tests_passed ? 0 : 1;
}
