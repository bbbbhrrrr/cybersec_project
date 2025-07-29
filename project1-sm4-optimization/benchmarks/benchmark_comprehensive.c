/*
 * SM4所有优化实现的综合性能基准测试
 * 包括基本实现、T-table、SIMD、AES-NI、GFNI、VPROLD和GCM模式
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "../src/common/sm4_common.h"
#include "../src/basic/sm4_basic.h"
#include "../src/ttable/sm4_ttable.h"
#include "../src/simd/sm4_simd.h"
#include "../src/aesni/sm4_aesni.h"
#include "../src/gfni/sm4_gfni.h"
#include "../src/vprold/sm4_vprold.h"
#include "../src/gcm/sm4_gcm.h"

// 测试配置
#define BENCHMARK_ITERATIONS 10000
#define BENCHMARK_BLOCK_SIZE 16
#define BENCHMARK_DATA_SIZE (BENCHMARK_ITERATIONS * BENCHMARK_BLOCK_SIZE)

// 测试密钥
static const uint8_t test_key[16] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
};

// 获取高精度时间戳
static double get_time(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// 计算吞吐量
static double calculate_throughput(size_t bytes, double time_seconds) {
    return (bytes / (1024.0 * 1024.0)) / time_seconds;  // MB/s
}

// 生成随机测试数据
static void generate_random_data(uint8_t *data, size_t size) {
    srand(12345);  // 固定种子保证可重现性
    for (size_t i = 0; i < size; i++) {
        data[i] = rand() & 0xFF;
    }
}

// 基本实现性能测试
static void benchmark_basic(const uint8_t *data, uint8_t *output, const uint32_t *round_keys) {
    printf("Testing Basic Implementation...\n");
    
    double start_time = get_time();
    
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        sm4_encrypt_basic(data + i * 16, output + i * 16, round_keys);
    }
    
    double end_time = get_time();
    double elapsed = end_time - start_time;
    double throughput = calculate_throughput(BENCHMARK_DATA_SIZE, elapsed);
    
    printf("  Time: %.3f seconds\n", elapsed);
    printf("  Throughput: %.2f MB/s\n", throughput);
    printf("  Blocks/sec: %.0f\n", BENCHMARK_ITERATIONS / elapsed);
    printf("\n");
}

// T-table实现性能测试
static void benchmark_ttable(const uint8_t *data, uint8_t *output, const uint32_t *round_keys) {
    printf("Testing T-table Implementation...\n");
    
    double start_time = get_time();
    
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        sm4_encrypt_ttable(data + i * 16, output + i * 16, round_keys);
    }
    
    double end_time = get_time();
    double elapsed = end_time - start_time;
    double throughput = calculate_throughput(BENCHMARK_DATA_SIZE, elapsed);
    
    printf("  Time: %.3f seconds\n", elapsed);
    printf("  Throughput: %.2f MB/s\n", throughput);
    printf("  Blocks/sec: %.0f\n", BENCHMARK_ITERATIONS / elapsed);
    printf("\n");
}

// SIMD实现性能测试
static void benchmark_simd(const uint8_t *data, uint8_t *output, const uint32_t *round_keys) {
    if (!sm4_simd_available()) {
        printf("SIMD Implementation: Not Available\n\n");
        return;
    }
    
    printf("Testing SIMD Implementation...\n");
    printf("  %s\n", sm4_simd_info());
    
    double start_time = get_time();
    
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        sm4_encrypt_block_simd(data + i * 16, output + i * 16, round_keys);
    }
    
    double end_time = get_time();
    double elapsed = end_time - start_time;
    double throughput = calculate_throughput(BENCHMARK_DATA_SIZE, elapsed);
    
    printf("  Time: %.3f seconds\n", elapsed);
    printf("  Throughput: %.2f MB/s\n", throughput);
    printf("  Blocks/sec: %.0f\n", BENCHMARK_ITERATIONS / elapsed);
    printf("\n");
}

// AES-NI实现性能测试
static void benchmark_aesni(const uint8_t *data, uint8_t *output, const uint32_t *round_keys) {
    if (!sm4_aesni_available()) {
        printf("AES-NI Implementation: Not Available\n\n");
        return;
    }
    
    printf("Testing AES-NI Implementation...\n");
    printf("  %s\n", sm4_aesni_info());
    
    // 单块性能测试
    double start_time = get_time();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        sm4_encrypt_block_aesni(data + i * 16, output + i * 16, round_keys);
    }
    double end_time = get_time();
    double elapsed = end_time - start_time;
    double throughput = calculate_throughput(BENCHMARK_DATA_SIZE, elapsed);
    
    printf("  Single-block mode:\n");
    printf("    Time: %.3f seconds\n", elapsed);
    printf("    Throughput: %.2f MB/s\n", throughput);
    printf("    Blocks/sec: %.0f\n", BENCHMARK_ITERATIONS / elapsed);
    
    // 4块并行性能测试
    int parallel_iterations = BENCHMARK_ITERATIONS / 4;
    start_time = get_time();
    for (int i = 0; i < parallel_iterations; i++) {
        sm4_encrypt_4blocks_aesni(data + i * 64, output + i * 64, round_keys);
    }
    end_time = get_time();
    elapsed = end_time - start_time;
    throughput = calculate_throughput(parallel_iterations * 64, elapsed);
    
    printf("  4-block parallel mode:\n");
    printf("    Time: %.3f seconds\n", elapsed);
    printf("    Throughput: %.2f MB/s\n", throughput);
    printf("    Blocks/sec: %.0f\n", (parallel_iterations * 4) / elapsed);
    printf("\n");
}

// GFNI实现性能测试
static void benchmark_gfni(const uint8_t *data, uint8_t *output, const uint32_t *round_keys) {
    if (!sm4_gfni_available()) {
        printf("GFNI Implementation: Not Available\n\n");
        return;
    }
    
    printf("Testing GFNI Implementation...\n");
    printf("  %s\n", sm4_gfni_info());
    
    // 单块性能测试
    double start_time = get_time();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        sm4_encrypt_block_gfni(data + i * 16, output + i * 16, round_keys);
    }
    double end_time = get_time();
    double elapsed = end_time - start_time;
    double throughput = calculate_throughput(BENCHMARK_DATA_SIZE, elapsed);
    
    printf("  Single-block mode:\n");
    printf("    Time: %.3f seconds\n", elapsed);
    printf("    Throughput: %.2f MB/s\n", throughput);
    printf("    Blocks/sec: %.0f\n", BENCHMARK_ITERATIONS / elapsed);
    
    // 8块并行性能测试
    int parallel_iterations = BENCHMARK_ITERATIONS / 8;
    start_time = get_time();
    for (int i = 0; i < parallel_iterations; i++) {
        sm4_encrypt_8blocks_gfni(data + i * 128, output + i * 128, round_keys);
    }
    end_time = get_time();
    elapsed = end_time - start_time;
    throughput = calculate_throughput(parallel_iterations * 128, elapsed);
    
    printf("  8-block AVX-512 mode:\n");
    printf("    Time: %.3f seconds\n", elapsed);
    printf("    Throughput: %.2f MB/s\n", throughput);
    printf("    Blocks/sec: %.0f\n", (parallel_iterations * 8) / elapsed);
    printf("\n");
}

// VPROLD实现性能测试
static void benchmark_vprold(const uint8_t *data, uint8_t *output, const uint32_t *round_keys) {
    if (!sm4_vprold_available()) {
        printf("VPROLD Implementation: Not Available\n\n");
        return;
    }
    
    printf("Testing VPROLD Implementation...\n");
    printf("  %s\n", sm4_vprold_info());
    
    // 单块性能测试
    double start_time = get_time();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        sm4_encrypt_block_vprold(data + i * 16, output + i * 16, round_keys);
    }
    double end_time = get_time();
    double elapsed = end_time - start_time;
    double throughput = calculate_throughput(BENCHMARK_DATA_SIZE, elapsed);
    
    printf("  Single-block mode:\n");
    printf("    Time: %.3f seconds\n", elapsed);
    printf("    Throughput: %.2f MB/s\n", throughput);
    printf("    Blocks/sec: %.0f\n", BENCHMARK_ITERATIONS / elapsed);
    
    // 16块并行性能测试
    int parallel_iterations = BENCHMARK_ITERATIONS / 16;
    start_time = get_time();
    for (int i = 0; i < parallel_iterations; i++) {
        sm4_encrypt_16blocks_vprold(data + i * 256, output + i * 256, round_keys);
    }
    end_time = get_time();
    elapsed = end_time - start_time;
    throughput = calculate_throughput(parallel_iterations * 256, elapsed);
    
    printf("  16-block parallel mode:\n");
    printf("    Time: %.3f seconds\n", elapsed);
    printf("    Throughput: %.2f MB/s\n", throughput);
    printf("    Blocks/sec: %.0f\n", (parallel_iterations * 16) / elapsed);
    printf("\n");
}

// GCM模式性能测试
static void benchmark_gcm(const uint8_t *data, size_t data_size) {
    printf("Testing SM4-GCM Mode...\n");
    
    const uint8_t iv[12] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 
                           0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b};
    const uint8_t aad[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                            0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
    
    uint8_t *ciphertext = malloc(data_size);
    uint8_t *decrypted = malloc(data_size);
    uint8_t tag[16];
    
    if (!ciphertext || !decrypted) {
        printf("  Memory allocation failed\n\n");
        free(ciphertext);
        free(decrypted);
        return;
    }
    
    // GCM加密性能测试
    double start_time = get_time();
    
    int result = sm4_gcm_encrypt(test_key, iv, 12, aad, 16, 
                                data, ciphertext, data_size, tag);
    
    double end_time = get_time();
    
    if (result != 0) {
        printf("  GCM encryption failed\n\n");
        free(ciphertext);
        free(decrypted);
        return;
    }
    
    double encrypt_time = end_time - start_time;
    double encrypt_throughput = calculate_throughput(data_size, encrypt_time);
    
    printf("  Encryption:\n");
    printf("    Time: %.3f seconds\n", encrypt_time);
    printf("    Throughput: %.2f MB/s\n", encrypt_throughput);
    
    // GCM解密性能测试
    start_time = get_time();
    
    result = sm4_gcm_decrypt(test_key, iv, 12, aad, 16, 
                            ciphertext, decrypted, data_size, tag);
    
    end_time = get_time();
    
    if (result != 0) {
        printf("  GCM decryption failed\n\n");
        free(ciphertext);
        free(decrypted);
        return;
    }
    
    double decrypt_time = end_time - start_time;
    double decrypt_throughput = calculate_throughput(data_size, decrypt_time);
    
    printf("  Decryption:\n");
    printf("    Time: %.3f seconds\n", decrypt_time);
    printf("    Throughput: %.2f MB/s\n", decrypt_throughput);
    printf("\n");
    
    free(ciphertext);
    free(decrypted);
}

// 比较所有实现的性能
static void performance_comparison(void) {
    printf("Performance Comparison Summary\n");
    printf("============================\n");
    
    // 这里可以添加性能比较的汇总统计
    printf("Implementation rankings (estimated based on optimization level):\n");
    printf("1. VPROLD (AVX-512 Vector Rotate) - Highest performance\n");
    printf("2. GFNI (Galois Field Instructions) - Very high performance\n");
    printf("3. AES-NI (AES New Instructions) - High performance\n");
    printf("4. SIMD (SSE/AVX) - Good performance\n");
    printf("5. T-table (Lookup tables) - Moderate performance\n");
    printf("6. Basic (Reference implementation) - Baseline performance\n");
    printf("\nNote: Actual performance depends on CPU capabilities and data size.\n");
    printf("GCM mode adds authentication overhead but provides integrity protection.\n\n");
}

int main(void) {
    printf("SM4 Comprehensive Performance Benchmark\n");
    printf("======================================\n");
    printf("Test configuration:\n");
    printf("  Block size: %d bytes\n", BENCHMARK_BLOCK_SIZE);
    printf("  Iterations: %d\n", BENCHMARK_ITERATIONS);
    printf("  Total data: %.2f MB\n\n", BENCHMARK_DATA_SIZE / (1024.0 * 1024.0));
    
    // 分配测试数据
    uint8_t *test_data = malloc(BENCHMARK_DATA_SIZE);
    uint8_t *output_data = malloc(BENCHMARK_DATA_SIZE);
    uint32_t round_keys[32];
    
    if (!test_data || !output_data) {
        printf("Memory allocation failed\n");
        free(test_data);
        free(output_data);
        return 1;
    }
    
    // 生成测试数据和密钥
    generate_random_data(test_data, BENCHMARK_DATA_SIZE);
    sm4_key_schedule(test_key, round_keys);
    
    // 执行所有基准测试
    benchmark_basic(test_data, output_data, round_keys);
    benchmark_ttable(test_data, output_data, round_keys);
    benchmark_simd(test_data, output_data, round_keys);
    benchmark_aesni(test_data, output_data, round_keys);
    benchmark_gfni(test_data, output_data, round_keys);
    benchmark_vprold(test_data, output_data, round_keys);
    benchmark_gcm(test_data, BENCHMARK_DATA_SIZE);
    
    // 性能比较总结
    performance_comparison();
    
    free(test_data);
    free(output_data);
    
    printf("Benchmark completed successfully!\n");
    return 0;
}
