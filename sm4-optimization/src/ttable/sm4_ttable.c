#include "../common/sm4.h"
#include <string.h>

// T-table optimization implementation
// Pre-computed tables that combine S-box substitution and linear transformation

// T-tables for the four positions in a 32-bit word
static uint32_t T0[256];
static uint32_t T1[256];
static uint32_t T2[256];
static uint32_t T3[256];

// T-tables for key expansion
static uint32_t TK0[256];
static uint32_t TK1[256];
static uint32_t TK2[256];
static uint32_t TK3[256];

// SM4 S-box
static const uint8_t SM4_SBOX[256] = {
    0xd6, 0x90, 0xe9, 0xfe, 0xcc, 0xe1, 0x3d, 0xb7, 0x16, 0xb6, 0x14, 0xc2, 0x28, 0xfb, 0x2c, 0x05,
    0x2b, 0x67, 0x9a, 0x76, 0x2a, 0xbe, 0x04, 0xc3, 0xaa, 0x44, 0x13, 0x26, 0x49, 0x86, 0x06, 0x99,
    0x9c, 0x42, 0x50, 0xf4, 0x91, 0xef, 0x98, 0x7a, 0x33, 0x54, 0x0b, 0x43, 0xed, 0xcf, 0xac, 0x62,
    0xe4, 0xb3, 0x1c, 0xa9, 0xc9, 0x08, 0xe8, 0x95, 0x80, 0xdf, 0x94, 0xfa, 0x75, 0x8f, 0x3f, 0xa6,
    0x47, 0x07, 0xa7, 0xfc, 0xf3, 0x73, 0x17, 0xba, 0x83, 0x59, 0x3c, 0x19, 0xe6, 0x85, 0x4f, 0xa8,
    0x68, 0x6b, 0x81, 0xb2, 0x71, 0x64, 0xda, 0x8b, 0xf8, 0xeb, 0x0f, 0x4b, 0x70, 0x56, 0x9d, 0x35,
    0x1e, 0x24, 0x0e, 0x5e, 0x63, 0x58, 0xd1, 0xa2, 0x25, 0x22, 0x7c, 0x3b, 0x01, 0x21, 0x78, 0x87,
    0xd4, 0x00, 0x46, 0x57, 0x9f, 0xd3, 0x27, 0x52, 0x4c, 0x36, 0x02, 0xe7, 0xa0, 0xc4, 0xc8, 0x9e,
    0xea, 0xbf, 0x8a, 0xd2, 0x40, 0xc7, 0x38, 0xb5, 0xa3, 0xf7, 0xf2, 0xce, 0xf9, 0x61, 0x15, 0xa1,
    0xe0, 0xae, 0x5d, 0xa4, 0x9b, 0x34, 0x1a, 0x55, 0xad, 0x93, 0x32, 0x30, 0xf5, 0x8c, 0xb1, 0xe3,
    0x1d, 0xf6, 0xe2, 0x2e, 0x82, 0x66, 0xca, 0x60, 0xc0, 0x29, 0x23, 0xab, 0x0d, 0x53, 0x4e, 0x6f,
    0xd5, 0xdb, 0x37, 0x45, 0xde, 0xfd, 0x8e, 0x2f, 0x03, 0xff, 0x6a, 0x72, 0x6d, 0x6c, 0x5b, 0x51,
    0x8d, 0x1b, 0xaf, 0x92, 0xbb, 0xdd, 0xbc, 0x7f, 0x11, 0xd9, 0x5c, 0x41, 0x1f, 0x10, 0x5a, 0xd8,
    0x0a, 0xc1, 0x31, 0x88, 0xa5, 0xcd, 0x7b, 0xbd, 0x2d, 0x74, 0xd0, 0x12, 0xb8, 0xe5, 0xb4, 0xb0,
    0x89, 0x69, 0x97, 0x4a, 0x0c, 0x96, 0x77, 0x7e, 0x65, 0xb9, 0xf1, 0x09, 0xc5, 0x6e, 0xc6, 0x84,
    0x18, 0xf0, 0x7d, 0xec, 0x3a, 0xdc, 0x4d, 0x20, 0x79, 0xee, 0x5f, 0x3e, 0xd7, 0xcb, 0x39, 0x48
};

// System parameter FK
static const uint32_t SM4_FK[4] = {
    0xa3b1bac6, 0x56aa3350, 0x677d9197, 0xb27022dc
};

// Fixed parameter CK
static const uint32_t SM4_CK[32] = {
    0x00070e15, 0x1c232a31, 0x383f464d, 0x545b6269,
    0x70777e85, 0x8c939aa1, 0xa8afb6bd, 0xc4cbd2d9,
    0xe0e7eef5, 0xfc030a11, 0x181f262d, 0x343b4249,
    0x50575e65, 0x6c737a81, 0x888f969d, 0xa4abb2b9,
    0xc0c7ced5, 0xdce3eaf1, 0xf8ff060d, 0x141b2229,
    0x30373e45, 0x4c535a61, 0x686f767d, 0x848b9299,
    0xa0a7aeb5, 0xbcc3cad1, 0xd8dfe6ed, 0xf4fb0209,
    0x10171e25, 0x2c333a41, 0x484f565d, 0x646b7279
};

// Utility macros
#define ROL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

// Initialize T-tables (called once)
static int tables_initialized = 0;

static void init_ttables(void) {
    if (tables_initialized) return;
    
    for (int i = 0; i < 256; i++) {
        uint32_t s = SM4_SBOX[i];
        
        // T-tables for encryption/decryption
        // Create 32-bit word with S-box value in each position
        uint32_t s0 = s << 24;  // S-box in MSB
        uint32_t s1 = s << 16;
        uint32_t s2 = s << 8;
        uint32_t s3 = s;        // S-box in LSB
        
        // Apply linear transformation L to each
        uint32_t L_s0 = s0 ^ ROL(s0, 2) ^ ROL(s0, 10) ^ ROL(s0, 18) ^ ROL(s0, 24);
        uint32_t L_s1 = s1 ^ ROL(s1, 2) ^ ROL(s1, 10) ^ ROL(s1, 18) ^ ROL(s1, 24);
        uint32_t L_s2 = s2 ^ ROL(s2, 2) ^ ROL(s2, 10) ^ ROL(s2, 18) ^ ROL(s2, 24);
        uint32_t L_s3 = s3 ^ ROL(s3, 2) ^ ROL(s3, 10) ^ ROL(s3, 18) ^ ROL(s3, 24);
        
        // Store results for each byte position
        T0[i] = L_s0;  // For MSB position
        T1[i] = L_s1;  // For second byte
        T2[i] = L_s2;  // For third byte  
        T3[i] = L_s3;  // For LSB position
        
        // T-tables for key expansion
        // Create 32-bit word with S-box value in each position
        uint32_t sk0 = s << 24;  // S-box in MSB
        uint32_t sk1 = s << 16;
        uint32_t sk2 = s << 8;
        uint32_t sk3 = s;        // S-box in LSB
        
        // Apply linear transformation L' to each
        uint32_t L_sk0 = sk0 ^ ROL(sk0, 13) ^ ROL(sk0, 23);
        uint32_t L_sk1 = sk1 ^ ROL(sk1, 13) ^ ROL(sk1, 23);
        uint32_t L_sk2 = sk2 ^ ROL(sk2, 13) ^ ROL(sk2, 23);
        uint32_t L_sk3 = sk3 ^ ROL(sk3, 13) ^ ROL(sk3, 23);
        
        // Store results for each byte position
        TK0[i] = L_sk0;  // For MSB position
        TK1[i] = L_sk1;  // For second byte
        TK2[i] = L_sk2;  // For third byte
        TK3[i] = L_sk3;  // For LSB position
    }
    
    tables_initialized = 1;
}

// Convert bytes to 32-bit word (big endian)
static uint32_t bytes_to_word(const uint8_t *bytes) {
    return ((uint32_t)bytes[0] << 24) |
           ((uint32_t)bytes[1] << 16) |
           ((uint32_t)bytes[2] << 8) |
           ((uint32_t)bytes[3]);
}

// Convert 32-bit word to bytes (big endian)
static void word_to_bytes(uint32_t word, uint8_t *bytes) {
    bytes[0] = (uint8_t)(word >> 24);
    bytes[1] = (uint8_t)(word >> 16);
    bytes[2] = (uint8_t)(word >> 8);
    bytes[3] = (uint8_t)(word);
}

// T-table based composite transformation T
static uint32_t sm4_T_ttable(uint32_t x) {
    return T0[(x >> 24) & 0xFF] ^
           T1[(x >> 16) & 0xFF] ^
           T2[(x >> 8) & 0xFF] ^
           T3[x & 0xFF];
}

// T-table based composite transformation T' for key expansion
static uint32_t sm4_T_prime_ttable(uint32_t x) {
    return TK0[(x >> 24) & 0xFF] ^
           TK1[(x >> 16) & 0xFF] ^
           TK2[(x >> 8) & 0xFF] ^
           TK3[x & 0xFF];
}

// Key expansion algorithm using T-tables
static void sm4_key_expansion_ttable(const uint8_t key[16], uint32_t rk[32]) {
    uint32_t K[4];
    uint32_t MK[4];
    
    init_ttables();
    
    // Convert key to words
    for (int i = 0; i < 4; i++) {
        MK[i] = bytes_to_word(key + 4 * i);
    }
    
    // Initialize K with MK XOR FK
    for (int i = 0; i < 4; i++) {
        K[i] = MK[i] ^ SM4_FK[i];
    }
    
    // Generate round keys
    for (int i = 0; i < 32; i++) {
        K[(i + 4) % 4] = K[i % 4] ^ sm4_T_prime_ttable(K[(i + 1) % 4] ^ K[(i + 2) % 4] ^ K[(i + 3) % 4] ^ SM4_CK[i]);
        rk[i] = K[(i + 4) % 4];
    }
}

// SM4 encryption/decryption using T-tables
static void sm4_crypt_ttable(const uint32_t rk[32], const uint8_t input[16], uint8_t output[16], int encrypt) {
    uint32_t X[4];
    
    init_ttables();
    
    // Convert input to words
    for (int i = 0; i < 4; i++) {
        X[i] = bytes_to_word(input + 4 * i);
    }
    
    // 32 rounds of transformation
    for (int i = 0; i < 32; i++) {
        int round_key_idx = encrypt ? i : (31 - i);
        uint32_t temp = X[0] ^ sm4_T_ttable(X[1] ^ X[2] ^ X[3] ^ rk[round_key_idx]);
        X[0] = X[1];
        X[1] = X[2];
        X[2] = X[3];
        X[3] = temp;
    }
    
    // Reverse transformation R
    uint32_t Y[4];
    Y[0] = X[3];
    Y[1] = X[2];
    Y[2] = X[1];
    Y[3] = X[0];
    
    // Convert output to bytes
    for (int i = 0; i < 4; i++) {
        word_to_bytes(Y[i], output + 4 * i);
    }
}

// Set encryption key
int sm4_set_encrypt_key(sm4_context_t *ctx, const uint8_t key[SM4_KEY_SIZE]) {
    if (!ctx || !key) {
        return -1;
    }
    
    sm4_key_expansion_ttable(key, ctx->rk);
    return 0;
}

// Set decryption key (same as encryption key for SM4)
int sm4_set_decrypt_key(sm4_context_t *ctx, const uint8_t key[SM4_KEY_SIZE]) {
    return sm4_set_encrypt_key(ctx, key);
}

// Encrypt a single block
int sm4_encrypt_block(const sm4_context_t *ctx, const uint8_t input[SM4_BLOCK_SIZE], uint8_t output[SM4_BLOCK_SIZE]) {
    if (!ctx || !input || !output) {
        return -1;
    }
    
    sm4_crypt_ttable(ctx->rk, input, output, 1);
    return 0;
}

// Decrypt a single block
int sm4_decrypt_block(const sm4_context_t *ctx, const uint8_t input[SM4_BLOCK_SIZE], uint8_t output[SM4_BLOCK_SIZE]) {
    if (!ctx || !input || !output) {
        return -1;
    }
    
    sm4_crypt_ttable(ctx->rk, input, output, 0);
    return 0;
}
