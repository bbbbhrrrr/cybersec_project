#include "../src/common/sm3_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

// 外部函数声明
void sm3_hash_optimized(const uint8_t *data, size_t len, uint8_t *digest);

#ifdef __AVX2__
void sm3_simd_hash_batch(const uint8_t *data[], const size_t lengths[], 
                         uint8_t digests[][SM3_DIGEST_SIZE], uint32_t count);
#endif

/**
 * 高精度计时函数
 */
static double get_time_in_seconds(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

/**
 * 生成随机测试数据
 */
static uint8_t* generate_random_data(size_t size, unsigned int seed) {
    uint8_t *data = malloc(size);
    if (!data) return NULL;
    
    srand(seed);
    for (size_t i = 0; i < size; i++) {
        data[i] = (uint8_t)(rand() % 256);
    }
    
    return data;
}

/**
 * 基础SM3性能测试
 */
static void benchmark_basic_sm3(void) {
    printf("=== Basic SM3 Implementation Benchmark ===\n");
    
    const size_t test_sizes[] = {
        64,      // 1 block
        128,     // 2 blocks  
        1024,    // 16 blocks
        4096,    // 64 blocks
        16384,   // 256 blocks
        65536,   // 1024 blocks
        262144,  // 4096 blocks
        1048576  // 16384 blocks (1MB)
    };
    
    const size_t num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    const int base_iterations = 10000;
    
    printf("| Data Size | Iterations | Time (s) | Throughput (MB/s) | Cycles/Byte |\n");
    printf("|-----------|------------|----------|-------------------|-------------|\n");
    
    for (size_t i = 0; i < num_sizes; i++) {
        size_t size = test_sizes[i];
        int iterations = base_iterations / (size / 1024 + 1); // 根据大小调整迭代次数
        if (iterations < 100) iterations = 100;
        
        uint8_t *test_data = generate_random_data(size, 12345);
        if (!test_data) continue;
        
        uint8_t hash[SM3_DIGEST_SIZE];
        
        // 预热
        for (int j = 0; j < 10; j++) {
            sm3_hash(test_data, size, hash);
        }
        
        // 实际测试
        double start_time = get_time_in_seconds();
        
        for (int iter = 0; iter < iterations; iter++) {
            sm3_hash(test_data, size, hash);
        }
        
        double end_time = get_time_in_seconds();
        double total_time = end_time - start_time;
        
        double total_bytes = (double)size * iterations;
        double throughput = total_bytes / (total_time * 1024 * 1024); // MB/s
        double cycles_per_byte = (total_time * 2.4e9) / total_bytes; // 假设2.4GHz CPU
        
        printf("| %8zu  | %9d  | %8.3f | %16.2f | %11.2f |\n",
               size, iterations, total_time, throughput, cycles_per_byte);
        
        free(test_data);
    }
    
    printf("\n");
}

/**
 * 优化版SM3性能测试
 */
static void benchmark_optimized_sm3(void) {
    printf("=== Optimized SM3 Implementation Benchmark ===\n");
    
    const size_t test_sizes[] = {
        64, 128, 1024, 4096, 16384, 65536, 262144, 1048576
    };
    
    const size_t num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    const int base_iterations = 10000;
    
    printf("| Data Size | Iterations | Time (s) | Throughput (MB/s) | Speedup |\n");
    printf("|-----------|------------|----------|-------------------|----------|\n");
    
    for (size_t i = 0; i < num_sizes; i++) {
        size_t size = test_sizes[i];
        int iterations = base_iterations / (size / 1024 + 1);
        if (iterations < 100) iterations = 100;
        
        uint8_t *test_data = generate_random_data(size, 12345);
        if (!test_data) continue;
        
        uint8_t hash1[SM3_DIGEST_SIZE], hash2[SM3_DIGEST_SIZE];
        
        // 测试基础版本
        double start_time = get_time_in_seconds();
        for (int iter = 0; iter < iterations; iter++) {
            sm3_hash(test_data, size, hash1);
        }
        double basic_time = get_time_in_seconds() - start_time;
        
        // 测试优化版本
        start_time = get_time_in_seconds();
        for (int iter = 0; iter < iterations; iter++) {
            sm3_hash_optimized(test_data, size, hash2);
        }
        double optimized_time = get_time_in_seconds() - start_time;
        
        // 验证结果一致性
        if (memcmp(hash1, hash2, SM3_DIGEST_SIZE) != 0) {
            printf("ERROR: Optimized version produces different results!\n");
            free(test_data);
            continue;
        }
        
        double total_bytes = (double)size * iterations;
        double throughput = total_bytes / (optimized_time * 1024 * 1024);
        double speedup = basic_time / optimized_time;
        
        printf("| %8zu  | %9d  | %8.3f | %16.2f | %7.2fx |\n",
               size, iterations, optimized_time, throughput, speedup);
        
        free(test_data);
    }
    
    printf("\n");
}

/**
 * SIMD版本性能测试
 */
static void benchmark_simd_sm3(void) {
    printf("=== SIMD SM3 Implementation Benchmark ===\n");
    
#ifdef __AVX2__
    const size_t test_sizes[] = {64, 128, 1024, 4096, 16384, 65536};
    const size_t num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    const int base_iterations = 2500; // 减少迭代次数因为是批处理
    const int batch_size = 4;
    
    printf("| Data Size | Iterations | Time (s) | Throughput (MB/s) | SIMD Speedup |\n");
    printf("|-----------|------------|----------|-------------------|---------------|\n");
    
    for (size_t i = 0; i < num_sizes; i++) {
        size_t size = test_sizes[i];
        int iterations = base_iterations / (size / 1024 + 1);
        if (iterations < 25) iterations = 25;
        
        // 准备批处理数据
        const uint8_t *data_ptrs[batch_size];
        size_t lengths[batch_size];
        uint8_t single_hashes[batch_size][SM3_DIGEST_SIZE];
        uint8_t simd_hashes[batch_size][SM3_DIGEST_SIZE];
        
        uint8_t *test_data[batch_size];
        for (int j = 0; j < batch_size; j++) {
            test_data[j] = generate_random_data(size, 12345 + j);
            data_ptrs[j] = test_data[j];
            lengths[j] = size;
        }
        
        // 测试单独计算
        double start_time = get_time_in_seconds();
        for (int iter = 0; iter < iterations; iter++) {
            for (int j = 0; j < batch_size; j++) {
                sm3_hash(test_data[j], size, single_hashes[j]);
            }
        }
        double single_time = get_time_in_seconds() - start_time;
        
        // 测试SIMD批处理
        start_time = get_time_in_seconds();
        for (int iter = 0; iter < iterations; iter++) {
            sm3_simd_hash_batch(data_ptrs, lengths, simd_hashes, batch_size);
        }
        double simd_time = get_time_in_seconds() - start_time;
        
        // 验证结果一致性
        int results_match = 1;
        for (int j = 0; j < batch_size; j++) {
            if (memcmp(single_hashes[j], simd_hashes[j], SM3_DIGEST_SIZE) != 0) {
                results_match = 0;
                break;
            }
        }
        
        if (!results_match) {
            printf("ERROR: SIMD version produces different results!\n");
        } else {
            double total_bytes = (double)size * iterations * batch_size;
            double throughput = total_bytes / (simd_time * 1024 * 1024);
            double speedup = single_time / simd_time;
            
            printf("| %8zu  | %9d  | %8.3f | %16.2f | %12.2fx |\n",
                   size, iterations, simd_time, throughput, speedup);
        }
        
        for (int j = 0; j < batch_size; j++) {
            free(test_data[j]);
        }
    }
#else
    printf("SIMD benchmark skipped: AVX2 not supported\n");
#endif
    
    printf("\n");
}

/**
 * 内存使用模式测试
 */
static void benchmark_memory_patterns(void) {
    printf("=== Memory Access Pattern Benchmark ===\n");
    
    const size_t data_size = 1024 * 1024; // 1MB
    const int iterations = 1000;
    
    printf("Testing different memory access patterns with 1MB data:\n\n");
    
    // 连续内存访问
    {
        uint8_t *data = generate_random_data(data_size, 54321);
        uint8_t hash[SM3_DIGEST_SIZE];
        
        double start_time = get_time_in_seconds();
        for (int i = 0; i < iterations; i++) {
            sm3_hash(data, data_size, hash);
        }
        double time_taken = get_time_in_seconds() - start_time;
        double throughput = (data_size * iterations) / (time_taken * 1024 * 1024);
        
        printf("Sequential access:  %.3f seconds, %.2f MB/s\n", time_taken, throughput);
        free(data);
    }
    
    // 分块内存访问
    {
        const size_t block_size = 4096;
        const size_t num_blocks = data_size / block_size;
        
        uint8_t **blocks = malloc(num_blocks * sizeof(uint8_t*));
        for (size_t i = 0; i < num_blocks; i++) {
            blocks[i] = generate_random_data(block_size, 54321 + i);
        }
        
        double start_time = get_time_in_seconds();
        for (int iter = 0; iter < iterations; iter++) {
            for (size_t i = 0; i < num_blocks; i++) {
                uint8_t hash[SM3_DIGEST_SIZE];
                sm3_hash(blocks[i], block_size, hash);
            }
        }
        double time_taken = get_time_in_seconds() - start_time;
        double throughput = (data_size * iterations) / (time_taken * 1024 * 1024);
        
        printf("Fragmented access:  %.3f seconds, %.2f MB/s\n", time_taken, throughput);
        
        for (size_t i = 0; i < num_blocks; i++) {
            free(blocks[i]);
        }
        free(blocks);
    }
    
    printf("\n");
}

/**
 * 并发性能测试
 */
static void benchmark_concurrent_hashing(void) {
    printf("=== Concurrent Hashing Benchmark ===\n");
    
    const size_t data_size = 64 * 1024; // 64KB per thread
    const int iterations = 100;
    const int num_threads[] = {1, 2, 4, 8};
    const size_t num_thread_configs = sizeof(num_threads) / sizeof(num_threads[0]);
    
    printf("| Threads | Time (s) | Total Throughput (MB/s) | Per-Thread (MB/s) |\n");
    printf("|---------|----------|-------------------------|-------------------|\n");
    
    for (size_t i = 0; i < num_thread_configs; i++) {
        int threads = num_threads[i];
        
        // 为每个线程准备数据
        uint8_t **thread_data = malloc(threads * sizeof(uint8_t*));
        for (int t = 0; t < threads; t++) {
            thread_data[t] = generate_random_data(data_size, 98765 + t);
        }
        
        double start_time = get_time_in_seconds();
        
        // 模拟并发（简单的串行执行，实际应用中会使用pthread等）
        for (int iter = 0; iter < iterations; iter++) {
            for (int t = 0; t < threads; t++) {
                uint8_t hash[SM3_DIGEST_SIZE];
                sm3_hash(thread_data[t], data_size, hash);
            }
        }
        
        double time_taken = get_time_in_seconds() - start_time;
        double total_bytes = (double)data_size * iterations * threads;
        double total_throughput = total_bytes / (time_taken * 1024 * 1024);
        double per_thread_throughput = total_throughput / threads;
        
        printf("| %6d  | %8.3f | %22.2f | %17.2f |\n",
               threads, time_taken, total_throughput, per_thread_throughput);
        
        for (int t = 0; t < threads; t++) {
            free(thread_data[t]);
        }
        free(thread_data);
    }
    
    printf("\n");
}

/**
 * 主基准测试函数
 */
int main(void) {
    printf("=== SM3 Comprehensive Performance Benchmark ===\n\n");
    
    printf("System Information:\n");
    printf("- Compiler: %s\n", __VERSION__);
    printf("- Architecture: %s\n", 
#ifdef __x86_64__
        "x86_64"
#elif defined(__i386__)
        "i386"
#elif defined(__aarch64__)
        "ARM64"
#else
        "Unknown"
#endif
    );
    printf("- SIMD Support: %s\n",
#ifdef __AVX2__
        "AVX2"
#elif defined(__AVX__)
        "AVX"
#elif defined(__SSE2__)
        "SSE2"
#else
        "None"
#endif
    );
    printf("\n");
    
    // 运行各项基准测试
    benchmark_basic_sm3();
    benchmark_optimized_sm3();
    benchmark_simd_sm3();
    benchmark_memory_patterns();
    benchmark_concurrent_hashing();
    
    printf("=== Benchmark Complete ===\n");
    printf("Note: Results may vary depending on CPU, memory, and system load.\n");
    printf("For production use, consider running multiple iterations and averaging results.\n");
    
    return 0;
}
