#include "../src/common/sm4.h"
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

// Benchmark single block encryption
void benchmark_single_block() {
    printf("=== Single Block Encryption Benchmark ===\n");
    
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
    
    printf("Encryption Performance:\n");
    printf("  Iterations: %d\n", BENCHMARK_ITERATIONS);
    printf("  Time: %.3f seconds\n", elapsed);
    printf("  Blocks/sec: %.0f\n", ops_per_sec);
    printf("  Throughput: %.2f MB/s\n", mbps);
    printf("  Latency: %.2f ns/block\n", (elapsed * 1e9) / BENCHMARK_ITERATIONS);
    
    // Benchmark decryption
    sm4_set_decrypt_key(&ctx, key);
    
    // Warmup
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        sm4_decrypt_block(&ctx, ciphertext, plaintext);
    }
    
    start_time = get_time();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        sm4_decrypt_block(&ctx, ciphertext, plaintext);
    }
    end_time = get_time();
    
    elapsed = end_time - start_time;
    ops_per_sec = BENCHMARK_ITERATIONS / elapsed;
    bytes_per_sec = ops_per_sec * 16;
    mbps = bytes_per_sec / (1024 * 1024);
    
    printf("\nDecryption Performance:\n");
    printf("  Iterations: %d\n", BENCHMARK_ITERATIONS);
    printf("  Time: %.3f seconds\n", elapsed);
    printf("  Blocks/sec: %.0f\n", ops_per_sec);
    printf("  Throughput: %.2f MB/s\n", mbps);
    printf("  Latency: %.2f ns/block\n", (elapsed * 1e9) / BENCHMARK_ITERATIONS);
}

// Benchmark bulk data encryption
void benchmark_bulk_data() {
    printf("\n=== Bulk Data Encryption Benchmark ===\n");
    
    sm4_context_t ctx;
    uint8_t key[16];
    uint8_t *plaintext = malloc(TEST_DATA_SIZE);
    uint8_t *ciphertext = malloc(TEST_DATA_SIZE);
    
    if (!plaintext || !ciphertext) {
        printf("Failed to allocate memory for bulk test\n");
        free(plaintext);
        free(ciphertext);
        return;
    }
    
    // Generate test data
    generate_test_data(key, 16);
    generate_test_data(plaintext, TEST_DATA_SIZE);
    
    // Setup
    sm4_set_encrypt_key(&ctx, key);
    
    // Benchmark bulk encryption
    size_t num_blocks = TEST_DATA_SIZE / 16;
    
    double start_time = get_time();
    for (size_t i = 0; i < num_blocks; i++) {
        sm4_encrypt_block(&ctx, plaintext + i * 16, ciphertext + i * 16);
    }
    double end_time = get_time();
    
    double elapsed = end_time - start_time;
    double bytes_per_sec = TEST_DATA_SIZE / elapsed;
    double mbps = bytes_per_sec / (1024 * 1024);
    
    printf("Bulk Encryption Performance:\n");
    printf("  Data size: %d bytes (%d blocks)\n", TEST_DATA_SIZE, (int)num_blocks);
    printf("  Time: %.3f seconds\n", elapsed);
    printf("  Throughput: %.2f MB/s\n", mbps);
    
    // Benchmark bulk decryption
    sm4_set_decrypt_key(&ctx, key);
    
    start_time = get_time();
    for (size_t i = 0; i < num_blocks; i++) {
        sm4_decrypt_block(&ctx, ciphertext + i * 16, plaintext + i * 16);
    }
    end_time = get_time();
    
    elapsed = end_time - start_time;
    bytes_per_sec = TEST_DATA_SIZE / elapsed;
    mbps = bytes_per_sec / (1024 * 1024);
    
    printf("\nBulk Decryption Performance:\n");
    printf("  Data size: %d bytes (%d blocks)\n", TEST_DATA_SIZE, (int)num_blocks);
    printf("  Time: %.3f seconds\n", elapsed);
    printf("  Throughput: %.2f MB/s\n", mbps);
    
    free(plaintext);
    free(ciphertext);
}

// Benchmark key setup
void benchmark_key_setup() {
    printf("\n=== Key Setup Benchmark ===\n");
    
    sm4_context_t ctx;
    uint8_t key[16];
    generate_test_data(key, 16);
    
    // Warmup
    for (int i = 0; i < WARMUP_ITERATIONS / 10; i++) {
        sm4_set_encrypt_key(&ctx, key);
    }
    
    // Benchmark key setup
    int key_setup_iterations = BENCHMARK_ITERATIONS / 100;
    
    double start_time = get_time();
    for (int i = 0; i < key_setup_iterations; i++) {
        sm4_set_encrypt_key(&ctx, key);
    }
    double end_time = get_time();
    
    double elapsed = end_time - start_time;
    double ops_per_sec = key_setup_iterations / elapsed;
    
    printf("Key Setup Performance:\n");
    printf("  Iterations: %d\n", key_setup_iterations);
    printf("  Time: %.3f seconds\n", elapsed);
    printf("  Key setups/sec: %.0f\n", ops_per_sec);
    printf("  Latency: %.2f μs/key setup\n", (elapsed * 1e6) / key_setup_iterations);
}

// Memory usage analysis
void analyze_memory_usage() {
    printf("\n=== Memory Usage Analysis ===\n");
    
    printf("SM4 Context size: %zu bytes\n", sizeof(sm4_context_t));
    printf("Round keys: %d × 4 = %zu bytes\n", SM4_ROUNDS, SM4_ROUNDS * 4);
    printf("Block size: %d bytes\n", SM4_BLOCK_SIZE);
    printf("Key size: %d bytes\n", SM4_KEY_SIZE);
}

int main() {
    printf("SM4 Basic Implementation Performance Benchmark\n");
    printf("==============================================\n");
    
    benchmark_single_block();
    benchmark_bulk_data();
    benchmark_key_setup();
    analyze_memory_usage();
    
    printf("\n=== Benchmark Summary ===\n");
    printf("Test completed successfully.\n");
    printf("Results show baseline performance for SM4 basic implementation.\n");
    printf("These metrics will be used as reference for optimization comparisons.\n");
    
    return 0;
}
