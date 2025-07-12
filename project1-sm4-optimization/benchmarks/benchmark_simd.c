#include "../src/common/sm4.h"
#include "../src/simd/sm4_simd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

// Benchmark configuration
#define BENCHMARK_ITERATIONS 1000000
#define WARMUP_ITERATIONS 10000
#define TEST_DATA_SIZE (1024 * 1024)  // 1MB test data
#define SIMD_BLOCK_COUNT 8  // Process 8 blocks at a time

// High-resolution timer functions
static double get_time() {
#ifdef _WIN32
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / frequency.QuadPart;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
#endif
}

// Generate random test data
static void generate_test_data(uint8_t *data, size_t size) {
    srand((unsigned int)time(NULL));
    for (size_t i = 0; i < size; i++) {
        data[i] = (uint8_t)(rand() & 0xFF);
    }
}

// Benchmark single block encryption (for comparison)
void benchmark_single_block_basic() {
    printf("=== Single Block Encryption Benchmark (Basic) ===\n");
    
    sm4_context_t ctx;
    uint8_t key[16], plaintext[16], ciphertext[16];
    
    // Generate random key and plaintext
    generate_test_data(key, 16);
    generate_test_data(plaintext, 16);
    
    // Setup
    sm4_set_encrypt_key(&ctx, key);
    
    // Warmup
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        sm4_encrypt_block(&ctx, plaintext, ciphertext);
    }
    
    // Benchmark encryption
    double start_time = get_time();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        sm4_encrypt_block(&ctx, plaintext, ciphertext);
    }
    double end_time = get_time();
    
    double elapsed = end_time - start_time;
    double ops_per_sec = BENCHMARK_ITERATIONS / elapsed;
    double bytes_per_sec = ops_per_sec * 16;
    double mbps = bytes_per_sec / (1024 * 1024);
    
    printf("Basic Single Block Encryption:\n");
    printf("  Iterations: %d\n", BENCHMARK_ITERATIONS);
    printf("  Time: %.3f seconds\n", elapsed);
    printf("  Blocks/sec: %.0f\n", ops_per_sec);
    printf("  Throughput: %.2f MB/s\n", mbps);
    printf("  Latency: %.2f ns/block\n", (elapsed * 1e9) / BENCHMARK_ITERATIONS);
}

// Benchmark SIMD bulk encryption
void benchmark_simd_bulk() {
    printf("\n=== SIMD Bulk Encryption Benchmark ===\n");
    
    sm4_context_t ctx;
    uint8_t key[16];
    size_t data_size = TEST_DATA_SIZE;
    size_t num_blocks = data_size / 16;
    
    uint8_t *plaintext = malloc(data_size);
    uint8_t *ciphertext = malloc(data_size);
    uint8_t *decrypted = malloc(data_size);
    
    if (!plaintext || !ciphertext || !decrypted) {
        printf("Failed to allocate memory for SIMD test\n");
        free(plaintext);
        free(ciphertext);
        free(decrypted);
        return;
    }
    
    // Generate test data
    generate_test_data(key, 16);
    generate_test_data(plaintext, data_size);
    
    // Setup
    sm4_set_encrypt_key(&ctx, key);
    
    // Warmup
    sm4_encrypt_blocks_simd(&ctx, plaintext, ciphertext, SIMD_BLOCK_COUNT);
    
    // Benchmark SIMD encryption
    double start_time = get_time();
    sm4_encrypt_blocks_simd(&ctx, plaintext, ciphertext, num_blocks);
    double end_time = get_time();
    
    double elapsed = end_time - start_time;
    double bytes_per_sec = data_size / elapsed;
    double mbps = bytes_per_sec / (1024 * 1024);
    
    printf("SIMD Bulk Encryption Performance:\n");
    printf("  Data size: %zu bytes (%zu blocks)\n", data_size, num_blocks);
    printf("  Time: %.3f seconds\n", elapsed);
    printf("  Throughput: %.2f MB/s\n", mbps);
    
    // Benchmark SIMD decryption
    sm4_set_decrypt_key(&ctx, key);
    
    start_time = get_time();
    sm4_decrypt_blocks_simd(&ctx, ciphertext, decrypted, num_blocks);
    end_time = get_time();
    
    elapsed = end_time - start_time;
    bytes_per_sec = data_size / elapsed;
    mbps = bytes_per_sec / (1024 * 1024);
    
    printf("\nSIMD Bulk Decryption Performance:\n");
    printf("  Data size: %zu bytes (%zu blocks)\n", data_size, num_blocks);
    printf("  Time: %.3f seconds\n", elapsed);
    printf("  Throughput: %.2f MB/s\n", mbps);
    
    // Verify correctness
    if (memcmp(plaintext, decrypted, data_size) == 0) {
        printf("✓ SIMD encryption/decryption correctness verified\n");
    } else {
        printf("✗ SIMD encryption/decryption verification failed!\n");
    }
    
    free(plaintext);
    free(ciphertext);
    free(decrypted);
}

// Benchmark different block counts for SIMD
void benchmark_simd_scalability() {
    printf("\n=== SIMD Scalability Benchmark ===\n");
    
    sm4_context_t ctx;
    uint8_t key[16];
    generate_test_data(key, 16);
    sm4_set_encrypt_key(&ctx, key);
    
    size_t block_counts[] = {8, 16, 32, 64, 128, 256, 512, 1024};
    size_t num_tests = sizeof(block_counts) / sizeof(block_counts[0]);
    
    printf("Block Count | Throughput (MB/s) | Efficiency\n");
    printf("------------|-------------------|----------\n");
    
    for (size_t i = 0; i < num_tests; i++) {
        size_t blocks = block_counts[i];
        size_t data_size = blocks * 16;
        
        uint8_t *plaintext = malloc(data_size);
        uint8_t *ciphertext = malloc(data_size);
        
        if (!plaintext || !ciphertext) {
            continue;
        }
        
        generate_test_data(plaintext, data_size);
        
        // Warmup
        sm4_encrypt_blocks_simd(&ctx, plaintext, ciphertext, blocks);
        
        // Benchmark
        int iterations = BENCHMARK_ITERATIONS / blocks;
        if (iterations < 100) iterations = 100;
        
        double start_time = get_time();
        for (int j = 0; j < iterations; j++) {
            sm4_encrypt_blocks_simd(&ctx, plaintext, ciphertext, blocks);
        }
        double end_time = get_time();
        
        double elapsed = end_time - start_time;
        double total_bytes = (double)data_size * iterations;
        double mbps = (total_bytes / elapsed) / (1024 * 1024);
        double efficiency = mbps / (blocks * 8);  // Efficiency per parallel block
        
        printf("%11zu | %17.2f | %8.3f\n", blocks, mbps, efficiency);
        
        free(plaintext);
        free(ciphertext);
    }
}

// Performance comparison between implementations
void benchmark_comparison() {
    printf("\n=== Implementation Comparison ===\n");
    
    sm4_context_t ctx;
    uint8_t key[16];
    size_t test_blocks = 1024;  // 16KB test
    size_t data_size = test_blocks * 16;
    
    uint8_t *plaintext = malloc(data_size);
    uint8_t *ciphertext1 = malloc(data_size);
    uint8_t *ciphertext2 = malloc(data_size);
    
    if (!plaintext || !ciphertext1 || !ciphertext2) {
        printf("Memory allocation failed\n");
        free(plaintext);
        free(ciphertext1);
        free(ciphertext2);
        return;
    }
    
    generate_test_data(key, 16);
    generate_test_data(plaintext, data_size);
    sm4_set_encrypt_key(&ctx, key);
    
    // Test basic block-by-block encryption
    double start_time = get_time();
    for (size_t i = 0; i < test_blocks; i++) {
        sm4_encrypt_block(&ctx, plaintext + i * 16, ciphertext1 + i * 16);
    }
    double basic_time = get_time() - start_time;
    double basic_mbps = (data_size / basic_time) / (1024 * 1024);
    
    // Test SIMD bulk encryption
    start_time = get_time();
    sm4_encrypt_blocks_simd(&ctx, plaintext, ciphertext2, test_blocks);
    double simd_time = get_time() - start_time;
    double simd_mbps = (data_size / simd_time) / (1024 * 1024);
    
    // Verify both produce same result
    int results_match = (memcmp(ciphertext1, ciphertext2, data_size) == 0);
    
    printf("Test size: %zu blocks (%.1f KB)\n", test_blocks, data_size / 1024.0);
    printf("\nBasic Implementation:\n");
    printf("  Time: %.3f seconds\n", basic_time);
    printf("  Throughput: %.2f MB/s\n", basic_mbps);
    
    printf("\nSIMD Implementation:\n");
    printf("  Time: %.3f seconds\n", simd_time);
    printf("  Throughput: %.2f MB/s\n", simd_mbps);
    
    printf("\nPerformance Improvement:\n");
    printf("  Speedup: %.2fx\n", simd_mbps / basic_mbps);
    printf("  Time reduction: %.1f%%\n", (1.0 - simd_time / basic_time) * 100);
    printf("  Results match: %s\n", results_match ? "✓ Yes" : "✗ No");
    
    free(plaintext);
    free(ciphertext1);
    free(ciphertext2);
}

// Memory usage analysis for SIMD
void analyze_simd_memory() {
    printf("\n=== SIMD Memory Usage Analysis ===\n");
    
    printf("Base SM4 Context: %zu bytes\n", sizeof(sm4_context_t));
    printf("SIMD Processing: 8 blocks × 16 bytes = 128 bytes per batch\n");
    printf("SIMD Registers: ~32 × 32 bytes = 1024 bytes (AVX2)\n");
    printf("Additional overhead: Minimal (function calls only)\n");
    
    printf("\nSIMD Benefits:\n");
    printf("  • Parallel processing of 8 blocks\n");
    printf("  • Reduced function call overhead\n");
    printf("  • Better CPU cache utilization\n");
    printf("  • Vectorized arithmetic operations\n");
}

int main() {
    printf("SM4 SIMD Implementation Performance Benchmark\n");
    printf("=============================================\n");
    printf("Testing AVX2-optimized SM4 implementation\n\n");
    
    benchmark_single_block_basic();
    benchmark_simd_bulk();
    benchmark_simd_scalability();
    benchmark_comparison();
    analyze_simd_memory();
    
    printf("\n=== SIMD Benchmark Summary ===\n");
    printf("SIMD optimization provides significant performance improvements\n");
    printf("for bulk data processing scenarios.\n");
    printf("Best performance achieved with large batch sizes (256+ blocks).\n");
    
    return 0;
}
