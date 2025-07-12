#include "../src/common/sm4.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Test vectors from GB/T 32907-2016
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

// Additional test vectors for comprehensive testing
static const uint8_t test_key2[16] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};

static const uint8_t test_plaintext2[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

int test_basic_encryption() {
    printf("Testing basic SM4 encryption...\n");
    
    sm4_context_t ctx;
    uint8_t ciphertext[16];
    uint8_t decrypted[16];
    
    // Set encryption key
    if (sm4_set_encrypt_key(&ctx, test_key) != 0) {
        printf("FAIL: Failed to set encryption key\n");
        return 0;
    }
    
    // Encrypt
    if (sm4_encrypt_block(&ctx, test_plaintext, ciphertext) != 0) {
        printf("FAIL: Encryption failed\n");
        return 0;
    }
    
    // Check result
    if (memcmp(ciphertext, expected_ciphertext, 16) != 0) {
        printf("FAIL: Ciphertext mismatch\n");
        printf("Expected: ");
        print_hex(expected_ciphertext, 16);
        printf("Got:      ");
        print_hex(ciphertext, 16);
        return 0;
    }
    
    // Set decryption key
    if (sm4_set_decrypt_key(&ctx, test_key) != 0) {
        printf("FAIL: Failed to set decryption key\n");
        return 0;
    }
    
    // Decrypt
    if (sm4_decrypt_block(&ctx, ciphertext, decrypted) != 0) {
        printf("FAIL: Decryption failed\n");
        return 0;
    }
    
    // Check if decryption matches original plaintext
    if (memcmp(decrypted, test_plaintext, 16) != 0) {
        printf("FAIL: Decrypted text doesn't match original\n");
        printf("Original: ");
        print_hex(test_plaintext, 16);
        printf("Decrypted:");
        print_hex(decrypted, 16);
        return 0;
    }
    
    printf("PASS: Basic encryption/decryption test\n");
    return 1;
}

int test_additional_vectors() {
    printf("Testing additional test vectors...\n");
    
    sm4_context_t ctx;
    uint8_t ciphertext[16];
    uint8_t decrypted[16];
    
    // Test with second key/plaintext pair
    if (sm4_set_encrypt_key(&ctx, test_key2) != 0) {
        printf("FAIL: Failed to set encryption key (test 2)\n");
        return 0;
    }
    
    if (sm4_encrypt_block(&ctx, test_plaintext2, ciphertext) != 0) {
        printf("FAIL: Encryption failed (test 2)\n");
        return 0;
    }
    
    if (sm4_set_decrypt_key(&ctx, test_key2) != 0) {
        printf("FAIL: Failed to set decryption key (test 2)\n");
        return 0;
    }
    
    if (sm4_decrypt_block(&ctx, ciphertext, decrypted) != 0) {
        printf("FAIL: Decryption failed (test 2)\n");
        return 0;
    }
    
    if (memcmp(decrypted, test_plaintext2, 16) != 0) {
        printf("FAIL: Decrypted text doesn't match original (test 2)\n");
        return 0;
    }
    
    printf("PASS: Additional test vectors\n");
    return 1;
}

int test_error_handling() {
    printf("Testing error handling...\n");
    
    sm4_context_t ctx;
    uint8_t buffer[16];
    
    // Test NULL pointers
    if (sm4_set_encrypt_key(NULL, test_key) == 0) {
        printf("FAIL: Should return error for NULL context\n");
        return 0;
    }
    
    if (sm4_set_encrypt_key(&ctx, NULL) == 0) {
        printf("FAIL: Should return error for NULL key\n");
        return 0;
    }
    
    if (sm4_encrypt_block(NULL, test_plaintext, buffer) == 0) {
        printf("FAIL: Should return error for NULL context\n");
        return 0;
    }
    
    if (sm4_encrypt_block(&ctx, NULL, buffer) == 0) {
        printf("FAIL: Should return error for NULL input\n");
        return 0;
    }
    
    if (sm4_encrypt_block(&ctx, test_plaintext, NULL) == 0) {
        printf("FAIL: Should return error for NULL output\n");
        return 0;
    }
    
    printf("PASS: Error handling test\n");
    return 1;
}

int test_key_scheduling() {
    printf("Testing key scheduling consistency...\n");
    
    sm4_context_t ctx1, ctx2;
    uint8_t ciphertext1[16], ciphertext2[16];
    
    // Set same key twice
    sm4_set_encrypt_key(&ctx1, test_key);
    sm4_set_encrypt_key(&ctx2, test_key);
    
    // Encrypt same plaintext
    sm4_encrypt_block(&ctx1, test_plaintext, ciphertext1);
    sm4_encrypt_block(&ctx2, test_plaintext, ciphertext2);
    
    // Results should be identical
    if (memcmp(ciphertext1, ciphertext2, 16) != 0) {
        printf("FAIL: Same key produces different results\n");
        return 0;
    }
    
    printf("PASS: Key scheduling consistency test\n");
    return 1;
}

void print_test_info() {
    printf("=== SM4 Basic Implementation Test Suite ===\n");
    printf("Test Key: ");
    print_hex(test_key, 16);
    printf("Test Plaintext: ");
    print_hex(test_plaintext, 16);
    printf("Expected Ciphertext: ");
    print_hex(expected_ciphertext, 16);
    printf("\n");
}

int main() {
    int tests_passed = 0;
    int total_tests = 4;
    
    print_test_info();
    
    if (test_basic_encryption()) tests_passed++;
    if (test_additional_vectors()) tests_passed++;
    if (test_error_handling()) tests_passed++;
    if (test_key_scheduling()) tests_passed++;
    
    printf("\n=== Test Results ===\n");
    printf("Passed: %d/%d tests\n", tests_passed, total_tests);
    
    if (tests_passed == total_tests) {
        printf("All tests PASSED! ✓\n");
        return 0;
    } else {
        printf("Some tests FAILED! ✗\n");
        return 1;
    }
}
