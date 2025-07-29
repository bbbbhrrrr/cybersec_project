/*
 * SM4优化版本综合测试
 * 测试基本实现、T-table、SIMD、AES-NI、GFNI、VPROLD和GCM模式
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "../src/common/sm4_common.h"
#include "../src/basic/sm4_basic.h"
#include "../src/ttable/sm4_ttable.h"
#include "../src/simd/sm4_simd.h"
#include "../src/aesni/sm4_aesni.h"
#include "../src/gfni/sm4_gfni.h"
#include "../src/vprold/sm4_vprold.h"
#include "../src/gcm/sm4_gcm.h"

// 测试向量
static const uint8_t test_key[16] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
};

static const uint8_t test_plaintext[16] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
};

static const uint8_t expected_ciphertext[16] = {
    0x68, 0x1e, 0xdf, 0x34, 0xd2, 0x06, 0x96, 0x5e,
    0x86, 0xb3, 0xe9, 0x4f, 0x53, 0x6e, 0x42, 0x46
};

// 辅助函数：打印十六进制数据
static void print_hex(const char *label, const uint8_t *data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
        if (i % 16 == 15) printf("\n    ");
        else if (i % 4 == 3) printf(" ");
    }
    printf("\n");
}

// 比较两个数组
static int compare_arrays(const uint8_t *a, const uint8_t *b, size_t len) {
    return memcmp(a, b, len) == 0;
}

// 测试基本实现
static int test_basic_implementation(void) {
    printf("Testing Basic Implementation...\n");
    
    uint32_t round_keys[32];
    uint8_t ciphertext[16], decrypted[16];
    
    // 密钥扩展
    if (sm4_key_schedule(test_key, round_keys) != 0) {
        printf("Basic key schedule failed\n");
        return 0;
    }
    
    // 加密
    if (sm4_encrypt_basic(test_plaintext, ciphertext, round_keys) != 0) {
        printf("Basic encryption failed\n");
        return 0;
    }
    
    // 验证密文
    if (!compare_arrays(ciphertext, expected_ciphertext, 16)) {
        printf("Basic encryption result mismatch\n");
        print_hex("Expected", expected_ciphertext, 16);
        print_hex("Got", ciphertext, 16);
        return 0;
    }
    
    // 解密
    if (sm4_decrypt_basic(ciphertext, decrypted, round_keys) != 0) {
        printf("Basic decryption failed\n");
        return 0;
    }
    
    // 验证明文
    if (!compare_arrays(decrypted, test_plaintext, 16)) {
        printf("Basic decryption result mismatch\n");
        return 0;
    }
    
    printf("Basic Implementation: PASSED\n\n");
    return 1;
}

// 测试T-table实现
static int test_ttable_implementation(void) {
    printf("Testing T-table Implementation...\n");
    
    uint32_t round_keys[32];
    uint8_t ciphertext[16], decrypted[16];
    
    sm4_key_schedule(test_key, round_keys);
    
    if (sm4_encrypt_ttable(test_plaintext, ciphertext, round_keys) != 0) {
        printf("T-table encryption failed\n");
        return 0;
    }
    
    if (!compare_arrays(ciphertext, expected_ciphertext, 16)) {
        printf("T-table encryption result mismatch\n");
        return 0;
    }
    
    if (sm4_decrypt_ttable(ciphertext, decrypted, round_keys) != 0) {
        printf("T-table decryption failed\n");
        return 0;
    }
    
    if (!compare_arrays(decrypted, test_plaintext, 16)) {
        printf("T-table decryption result mismatch\n");
        return 0;
    }
    
    printf("T-table Implementation: PASSED\n\n");
    return 1;
}

// 测试SIMD实现
static int test_simd_implementation(void) {
    printf("Testing SIMD Implementation...\n");
    
    if (!sm4_simd_available()) {
        printf("SIMD not available, skipping test\n\n");
        return 1;
    }
    
    uint32_t round_keys[32];
    uint8_t ciphertext[16], decrypted[16];
    
    sm4_key_schedule(test_key, round_keys);
    
    if (sm4_encrypt_block_simd(test_plaintext, ciphertext, round_keys) != 0) {
        printf("SIMD encryption failed\n");
        return 0;
    }
    
    if (!compare_arrays(ciphertext, expected_ciphertext, 16)) {
        printf("SIMD encryption result mismatch\n");
        return 0;
    }
    
    if (sm4_decrypt_block_simd(ciphertext, decrypted, round_keys) != 0) {
        printf("SIMD decryption failed\n");
        return 0;
    }
    
    if (!compare_arrays(decrypted, test_plaintext, 16)) {
        printf("SIMD decryption result mismatch\n");
        return 0;
    }
    
    printf("SIMD Implementation: PASSED\n");
    printf("SIMD Info: %s\n\n", sm4_simd_info());
    return 1;
}

// 测试AES-NI实现
static int test_aesni_implementation(void) {
    printf("Testing AES-NI Implementation...\n");
    
    if (!sm4_aesni_available()) {
        printf("AES-NI not available, skipping test\n\n");
        return 1;
    }
    
    uint32_t round_keys[32];
    uint8_t ciphertext[16], decrypted[16];
    
    sm4_key_schedule(test_key, round_keys);
    
    if (sm4_encrypt_block_aesni(test_plaintext, ciphertext, round_keys) != 0) {
        printf("AES-NI encryption failed\n");
        return 0;
    }
    
    if (!compare_arrays(ciphertext, expected_ciphertext, 16)) {
        printf("AES-NI encryption result mismatch\n");
        return 0;
    }
    
    if (sm4_decrypt_block_aesni(ciphertext, decrypted, round_keys) != 0) {
        printf("AES-NI decryption failed\n");
        return 0;
    }
    
    if (!compare_arrays(decrypted, test_plaintext, 16)) {
        printf("AES-NI decryption result mismatch\n");
        return 0;
    }
    
    printf("AES-NI Implementation: PASSED\n");
    printf("AES-NI Info: %s\n\n", sm4_aesni_info());
    return 1;
}

// 测试GFNI实现
static int test_gfni_implementation(void) {
    printf("Testing GFNI Implementation...\n");
    
    if (!sm4_gfni_available()) {
        printf("GFNI not available, skipping test\n\n");
        return 1;
    }
    
    uint32_t round_keys[32];
    uint8_t ciphertext[16], decrypted[16];
    
    sm4_key_schedule(test_key, round_keys);
    
    if (sm4_encrypt_block_gfni(test_plaintext, ciphertext, round_keys) != 0) {
        printf("GFNI encryption failed\n");
        return 0;
    }
    
    if (!compare_arrays(ciphertext, expected_ciphertext, 16)) {
        printf("GFNI encryption result mismatch\n");
        return 0;
    }
    
    if (sm4_decrypt_block_gfni(ciphertext, decrypted, round_keys) != 0) {
        printf("GFNI decryption failed\n");
        return 0;
    }
    
    if (!compare_arrays(decrypted, test_plaintext, 16)) {
        printf("GFNI decryption result mismatch\n");
        return 0;
    }
    
    printf("GFNI Implementation: PASSED\n");
    printf("GFNI Info: %s\n\n", sm4_gfni_info());
    return 1;
}

// 测试VPROLD实现
static int test_vprold_implementation(void) {
    printf("Testing VPROLD Implementation...\n");
    
    if (!sm4_vprold_available()) {
        printf("VPROLD not available, skipping test\n\n");
        return 1;
    }
    
    uint32_t round_keys[32];
    uint8_t ciphertext[16], decrypted[16];
    
    sm4_key_schedule(test_key, round_keys);
    
    if (sm4_encrypt_block_vprold(test_plaintext, ciphertext, round_keys) != 0) {
        printf("VPROLD encryption failed\n");
        return 0;
    }
    
    if (!compare_arrays(ciphertext, expected_ciphertext, 16)) {
        printf("VPROLD encryption result mismatch\n");
        return 0;
    }
    
    if (sm4_decrypt_block_vprold(ciphertext, decrypted, round_keys) != 0) {
        printf("VPROLD decryption failed\n");
        return 0;
    }
    
    if (!compare_arrays(decrypted, test_plaintext, 16)) {
        printf("VPROLD decryption result mismatch\n");
        return 0;
    }
    
    printf("VPROLD Implementation: PASSED\n");
    printf("VPROLD Info: %s\n\n", sm4_vprold_info());
    return 1;
}

// 测试GCM模式
static int test_gcm_mode(void) {
    printf("Testing SM4-GCM Mode...\n");
    
    const uint8_t iv[12] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 
                           0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b};
    const uint8_t aad[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                            0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
    const char *test_msg = "Hello, SM4-GCM!";
    size_t msg_len = strlen(test_msg);
    
    uint8_t ciphertext[32];
    uint8_t decrypted[32];
    uint8_t tag[16];
    
    // GCM加密
    int result = sm4_gcm_encrypt(test_key, iv, 12, aad, 16, 
                                (uint8_t*)test_msg, ciphertext, msg_len, tag);
    if (result != 0) {
        printf("GCM encryption failed with error %d\n", result);
        return 0;
    }
    
    printf("GCM Encryption successful\n");
    print_hex("Ciphertext", ciphertext, msg_len);
    print_hex("Tag", tag, 16);
    
    // GCM解密
    result = sm4_gcm_decrypt(test_key, iv, 12, aad, 16, 
                            ciphertext, decrypted, msg_len, tag);
    if (result != 0) {
        printf("GCM decryption failed with error %d\n", result);
        return 0;
    }
    
    decrypted[msg_len] = '\0';
    if (strcmp((char*)decrypted, test_msg) != 0) {
        printf("GCM decryption result mismatch\n");
        printf("Expected: %s\n", test_msg);
        printf("Got: %s\n", (char*)decrypted);
        return 0;
    }
    
    printf("SM4-GCM Mode: PASSED\n\n");
    return 1;
}

// 性能测试
static void performance_test(void) {
    printf("Performance Testing...\n");
    
    const int test_blocks = 10000;
    const size_t test_size = test_blocks * 16;
    uint8_t *test_data = malloc(test_size);
    uint8_t *output_data = malloc(test_size);
    uint32_t round_keys[32];
    
    // 生成随机测试数据
    srand((unsigned int)time(NULL));
    for (size_t i = 0; i < test_size; i++) {
        test_data[i] = rand() & 0xFF;
    }
    
    sm4_key_schedule(test_key, round_keys);
    
    clock_t start, end;
    double cpu_time_used;
    
    // 测试基本实现
    start = clock();
    for (int i = 0; i < test_blocks; i++) {
        sm4_encrypt_basic(test_data + i * 16, output_data + i * 16, round_keys);
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Basic: %.2f MB/s\n", (test_size / (1024.0 * 1024.0)) / cpu_time_used);
    
    // 测试T-table实现
    start = clock();
    for (int i = 0; i < test_blocks; i++) {
        sm4_encrypt_ttable(test_data + i * 16, output_data + i * 16, round_keys);
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("T-table: %.2f MB/s\n", (test_size / (1024.0 * 1024.0)) / cpu_time_used);
    
    // 测试SIMD实现（如果支持）
    if (sm4_simd_available()) {
        start = clock();
        for (int i = 0; i < test_blocks; i++) {
            sm4_encrypt_block_simd(test_data + i * 16, output_data + i * 16, round_keys);
        }
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("SIMD: %.2f MB/s\n", (test_size / (1024.0 * 1024.0)) / cpu_time_used);
    }
    
    // 测试AES-NI实现（如果支持）
    if (sm4_aesni_available()) {
        start = clock();
        for (int i = 0; i < test_blocks; i++) {
            sm4_encrypt_block_aesni(test_data + i * 16, output_data + i * 16, round_keys);
        }
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("AES-NI: %.2f MB/s\n", (test_size / (1024.0 * 1024.0)) / cpu_time_used);
    }
    
    free(test_data);
    free(output_data);
    printf("\n");
}

int main(void) {
    printf("SM4 Optimization Test Suite\n");
    printf("===========================\n\n");
    
    int passed = 0, total = 0;
    
    // 功能测试
    total++; if (test_basic_implementation()) passed++;
    total++; if (test_ttable_implementation()) passed++;
    total++; if (test_simd_implementation()) passed++;
    total++; if (test_aesni_implementation()) passed++;
    total++; if (test_gfni_implementation()) passed++;
    total++; if (test_vprold_implementation()) passed++;
    total++; if (test_gcm_mode()) passed++;
    
    // 性能测试
    performance_test();
    
    printf("Test Results: %d/%d passed\n", passed, total);
    
    if (passed == total) {
        printf("All tests PASSED!\n");
        return 0;
    } else {
        printf("Some tests FAILED!\n");
        return 1;
    }
}
