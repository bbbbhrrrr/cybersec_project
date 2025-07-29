#include "../src/common/sm3_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

// 外部函数声明
void sm3_hash_optimized(const uint8_t *data, size_t len, uint8_t *digest);
void demonstrate_length_extension_attack(void);

// SIMD函数声明
#ifdef __AVX2__
void sm3_simd_hash_batch(const uint8_t *data[], const size_t lengths[], 
                         uint8_t digests[][SM3_DIGEST_SIZE], uint32_t count);
#endif

// Merkle树函数声明
typedef struct merkle_tree merkle_tree_t;
merkle_tree_t* build_large_merkle_tree(size_t leaf_count);
void demonstrate_merkle_proofs(merkle_tree_t *tree);

/**
 * SM3测试向量（来自国家标准GM/T 0004-2012）
 */
typedef struct {
    const char *message;
    const char *expected_hash;
} sm3_test_vector_t;

static const sm3_test_vector_t test_vectors[] = {
    {
        "abc",
        "66c7f0f462eeedd9d1f2d46bdc10e4e24167c4875cf2f7a2297da02b8f4ba8e0"
    },
    {
        "abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd",
        "debe9ff92275b8a138604889c18e5a4d6fdb70e5387e5765293dcba39c0c5732"
    },
    {
        "",
        "1ab21d8355cfa17f8e61194831e81a8f22bec8c728fefb747ed035eb5082aa2b"
    },
    {
        "a",
        "623476ac18f65a2909e43c7fec61b49c7e764a91a18ccb82f1917a29c86c5e88"
    }
};

static const size_t num_test_vectors = sizeof(test_vectors) / sizeof(test_vectors[0]);

/**
 * 辅助函数：十六进制字符串转字节数组
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
 * 辅助函数：字节数组转十六进制字符串
 */
static void bytes_to_hex(const uint8_t *bytes, size_t len, char *hex) {
    for (size_t i = 0; i < len; i++) {
        sprintf(hex + i*2, "%02x", bytes[i]);
    }
    hex[len*2] = '\0';
}

/**
 * 测试基础SM3实现
 */
static int test_basic_sm3(void) {
    printf("Testing basic SM3 implementation...\n");
    
    int passed = 0;
    for (size_t i = 0; i < num_test_vectors; i++) {
        uint8_t computed_hash[SM3_DIGEST_SIZE];
        uint8_t expected_hash[SM3_DIGEST_SIZE];
        char computed_hex[SM3_DIGEST_SIZE * 2 + 1];
        
        // 计算哈希值
        sm3_hash((const uint8_t*)test_vectors[i].message, 
                strlen(test_vectors[i].message), computed_hash);
        
        // 转换期望哈希值
        hex_to_bytes(test_vectors[i].expected_hash, expected_hash);
        
        // 比较结果
        if (memcmp(computed_hash, expected_hash, SM3_DIGEST_SIZE) == 0) {
            printf("  Test %zu: PASSED\n", i + 1);
            passed++;
        } else {
            bytes_to_hex(computed_hash, SM3_DIGEST_SIZE, computed_hex);
            printf("  Test %zu: FAILED\n", i + 1);
            printf("    Message: %s\n", test_vectors[i].message);
            printf("    Expected: %s\n", test_vectors[i].expected_hash);
            printf("    Computed: %s\n", computed_hex);
        }
    }
    
    printf("Basic SM3: %d/%zu tests passed\n\n", passed, num_test_vectors);
    return passed == (int)num_test_vectors;
}

/**
 * 测试优化版SM3实现
 */
static int test_optimized_sm3(void) {
    printf("Testing optimized SM3 implementation...\n");
    
    int passed = 0;
    for (size_t i = 0; i < num_test_vectors; i++) {
        uint8_t computed_hash[SM3_DIGEST_SIZE];
        uint8_t expected_hash[SM3_DIGEST_SIZE];
        char computed_hex[SM3_DIGEST_SIZE * 2 + 1];
        
        // 计算哈希值
        sm3_hash_optimized((const uint8_t*)test_vectors[i].message, 
                          strlen(test_vectors[i].message), computed_hash);
        
        // 转换期望哈希值
        hex_to_bytes(test_vectors[i].expected_hash, expected_hash);
        
        // 比较结果
        if (memcmp(computed_hash, expected_hash, SM3_DIGEST_SIZE) == 0) {
            printf("  Test %zu: PASSED\n", i + 1);
            passed++;
        } else {
            bytes_to_hex(computed_hash, SM3_DIGEST_SIZE, computed_hex);
            printf("  Test %zu: FAILED\n", i + 1);
            printf("    Message: %s\n", test_vectors[i].message);
            printf("    Expected: %s\n", test_vectors[i].expected_hash);
            printf("    Computed: %s\n", computed_hex);
        }
    }
    
    printf("Optimized SM3: %d/%zu tests passed\n\n", passed, num_test_vectors);
    return passed == (int)num_test_vectors;
}

/**
 * 测试SIMD版SM3实现
 */
static int test_simd_sm3(void) {
    printf("Testing SIMD SM3 implementation...\n");
    
#ifdef __AVX2__
    const uint8_t *data[4] = {
        (const uint8_t*)test_vectors[0].message,
        (const uint8_t*)test_vectors[1].message,
        (const uint8_t*)test_vectors[2].message,
        (const uint8_t*)test_vectors[3].message
    };
    
    size_t lengths[4] = {
        strlen(test_vectors[0].message),
        strlen(test_vectors[1].message),
        strlen(test_vectors[2].message),
        strlen(test_vectors[3].message)
    };
    
    uint8_t computed_hashes[4][SM3_DIGEST_SIZE];
    sm3_simd_hash_batch(data, lengths, computed_hashes, 4);
    
    int passed = 0;
    for (int i = 0; i < 4; i++) {
        uint8_t expected_hash[SM3_DIGEST_SIZE];
        hex_to_bytes(test_vectors[i].expected_hash, expected_hash);
        
        if (memcmp(computed_hashes[i], expected_hash, SM3_DIGEST_SIZE) == 0) {
            printf("  SIMD Test %d: PASSED\n", i + 1);
            passed++;
        } else {
            char computed_hex[SM3_DIGEST_SIZE * 2 + 1];
            bytes_to_hex(computed_hashes[i], SM3_DIGEST_SIZE, computed_hex);
            printf("  SIMD Test %d: FAILED\n", i + 1);
            printf("    Expected: %s\n", test_vectors[i].expected_hash);
            printf("    Computed: %s\n", computed_hex);
        }
    }
    
    printf("SIMD SM3: %d/4 tests passed\n\n", passed);
    return passed == 4;
#else
    printf("SIMD SM3: AVX2 not supported, skipping tests\n\n");
    return 1; // 认为通过，因为不支持SIMD
#endif
}

/**
 * 性能基准测试
 */
static void benchmark_sm3_implementations(void) {
    printf("=== SM3 Performance Benchmark ===\n");
    
    const size_t test_sizes[] = {1024, 4096, 16384, 65536, 262144}; // 1KB to 256KB
    const size_t num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    const int iterations = 1000;
    
    for (size_t i = 0; i < num_sizes; i++) {
        size_t size = test_sizes[i];
        uint8_t *test_data = malloc(size);
        
        // 生成随机测试数据
        for (size_t j = 0; j < size; j++) {
            test_data[j] = (uint8_t)(rand() % 256);
        }
        
        printf("\nTesting with %zu bytes (%d iterations):\n", size, iterations);
        
        // 基础版本基准测试
        {
            uint8_t hash[SM3_DIGEST_SIZE];
            clock_t start = clock();
            
            for (int iter = 0; iter < iterations; iter++) {
                sm3_hash(test_data, size, hash);
            }
            
            clock_t end = clock();
            double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
            double throughput = (size * iterations) / (time_taken * 1024 * 1024); // MB/s
            
            printf("  Basic SM3:     %.3f seconds, %.2f MB/s\n", time_taken, throughput);
        }
        
        // 优化版本基准测试
        {
            uint8_t hash[SM3_DIGEST_SIZE];
            clock_t start = clock();
            
            for (int iter = 0; iter < iterations; iter++) {
                sm3_hash_optimized(test_data, size, hash);
            }
            
            clock_t end = clock();
            double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
            double throughput = (size * iterations) / (time_taken * 1024 * 1024); // MB/s
            
            printf("  Optimized SM3: %.3f seconds, %.2f MB/s\n", time_taken, throughput);
        }
        
        free(test_data);
    }
    
    printf("\n");
}

/**
 * 主测试函数
 */
int main(void) {
    printf("=== SM3 Comprehensive Test Suite ===\n\n");
    
    int all_passed = 1;
    
    // 基本功能测试
    if (!test_basic_sm3()) {
        all_passed = 0;
    }
    
    if (!test_optimized_sm3()) {
        all_passed = 0;
    }
    
    if (!test_simd_sm3()) {
        all_passed = 0;
    }
    
    // 性能基准测试
    benchmark_sm3_implementations();
    
    // 长度扩展攻击演示
    printf("=== Length Extension Attack Test ===\n");
    demonstrate_length_extension_attack();
    printf("\n");
    
    // Merkle树测试
    printf("=== Merkle Tree Test ===\n");
    printf("Building Merkle tree with 100,000 leaves...\n");
    merkle_tree_t *tree = build_large_merkle_tree(100000);
    if (tree) {
        demonstrate_merkle_proofs(tree);
        printf("Merkle tree test completed successfully\n");
    } else {
        printf("Merkle tree test failed\n");
        all_passed = 0;
    }
    
    // 测试总结
    printf("\n=== Test Summary ===\n");
    if (all_passed) {
        printf("All tests PASSED! SM3 implementation is correct.\n");
    } else {
        printf("Some tests FAILED! Please check the implementation.\n");
    }
    
    return all_passed ? 0 : 1;
}
