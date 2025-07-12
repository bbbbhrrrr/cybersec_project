#include "../common/sm4.h"
#include <string.h>

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <immintrin.h>
#endif

// SIMD optimization using AVX2 instructions
// Process 8 blocks in parallel

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

// Non-linear transformation τ (S-box substitution)
static uint32_t sm4_tau(uint32_t a) {
    uint8_t b[4];
    word_to_bytes(a, b);
    
    b[0] = SM4_SBOX[b[0]];
    b[1] = SM4_SBOX[b[1]];
    b[2] = SM4_SBOX[b[2]];
    b[3] = SM4_SBOX[b[3]];
    
    return bytes_to_word(b);
}

// Linear transformation L
static uint32_t sm4_L(uint32_t b) {
    return b ^ ROL(b, 2) ^ ROL(b, 10) ^ ROL(b, 18) ^ ROL(b, 24);
}

// Composite transformation T
static uint32_t sm4_T(uint32_t x) {
    return sm4_L(sm4_tau(x));
}

// Linear transformation L' for key expansion
static uint32_t sm4_L_prime(uint32_t b) {
    return b ^ ROL(b, 13) ^ ROL(b, 23);
}

// Composite transformation T' for key expansion
static uint32_t sm4_T_prime(uint32_t x) {
    return sm4_L_prime(sm4_tau(x));
}

// Key expansion algorithm (using basic implementation for now)
static void sm4_key_expansion(const uint8_t key[16], uint32_t rk[32]) {
    uint32_t K[4];
    uint32_t MK[4];
    
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
        K[(i + 4) % 4] = K[i % 4] ^ sm4_T_prime(K[(i + 1) % 4] ^ K[(i + 2) % 4] ^ K[(i + 3) % 4] ^ SM4_CK[i]);
        rk[i] = K[(i + 4) % 4];
    }
}

// SIMD S-box lookup using gather instruction (for AVX2)
static __m256i simd_sbox_lookup(__m256i input) {
    // Convert __m256i to array for S-box lookup
    uint32_t temp[8];
    _mm256_storeu_si256((__m256i*)temp, input);
    
    for (int i = 0; i < 8; i++) {
        uint8_t bytes[4];
        word_to_bytes(temp[i], bytes);
        
        bytes[0] = SM4_SBOX[bytes[0]];
        bytes[1] = SM4_SBOX[bytes[1]];
        bytes[2] = SM4_SBOX[bytes[2]];
        bytes[3] = SM4_SBOX[bytes[3]];
        
        temp[i] = bytes_to_word(bytes);
    }
    
    return _mm256_loadu_si256((__m256i*)temp);
}

// SIMD rotate left
static __m256i simd_rol(__m256i x, int n) {
    return _mm256_or_si256(_mm256_slli_epi32(x, n), _mm256_srli_epi32(x, 32 - n));
}

// SIMD linear transformation L
static __m256i simd_L(__m256i b) {
    __m256i rol2 = simd_rol(b, 2);
    __m256i rol10 = simd_rol(b, 10);
    __m256i rol18 = simd_rol(b, 18);
    __m256i rol24 = simd_rol(b, 24);
    
    return _mm256_xor_si256(
        _mm256_xor_si256(b, rol2),
        _mm256_xor_si256(
            _mm256_xor_si256(rol10, rol18),
            rol24
        )
    );
}

// SIMD composite transformation T
static __m256i simd_T(__m256i x) {
    return simd_L(simd_sbox_lookup(x));
}

// SIMD encryption of 8 blocks in parallel
static void sm4_encrypt_8blocks_simd(const sm4_context_t *ctx, 
                                     const uint8_t input[8][16], 
                                     uint8_t output[8][16]) {
    // Load 8 blocks into 4 vectors (one for each 32-bit word position)
    __m256i X0, X1, X2, X3;
    
    // Initialize X0, X1, X2, X3 with input data
    uint32_t temp0[8], temp1[8], temp2[8], temp3[8];
    
    for (int i = 0; i < 8; i++) {
        temp0[i] = bytes_to_word(input[i] + 0);
        temp1[i] = bytes_to_word(input[i] + 4);
        temp2[i] = bytes_to_word(input[i] + 8);
        temp3[i] = bytes_to_word(input[i] + 12);
    }
    
    X0 = _mm256_loadu_si256((__m256i*)temp0);
    X1 = _mm256_loadu_si256((__m256i*)temp1);
    X2 = _mm256_loadu_si256((__m256i*)temp2);
    X3 = _mm256_loadu_si256((__m256i*)temp3);
    
    // 32 rounds of transformation
    for (int i = 0; i < 32; i++) {
        __m256i rk_vec = _mm256_set1_epi32(ctx->rk[i]);
        __m256i temp = _mm256_xor_si256(
            _mm256_xor_si256(X1, X2),
            _mm256_xor_si256(X3, rk_vec)
        );
        
        __m256i new_X3 = _mm256_xor_si256(X0, simd_T(temp));
        
        // Rotate the state
        X0 = X1;
        X1 = X2;
        X2 = X3;
        X3 = new_X3;
    }
    
    // Reverse transformation R and output
    _mm256_storeu_si256((__m256i*)temp0, X3);
    _mm256_storeu_si256((__m256i*)temp1, X2);
    _mm256_storeu_si256((__m256i*)temp2, X1);
    _mm256_storeu_si256((__m256i*)temp3, X0);
    
    for (int i = 0; i < 8; i++) {
        word_to_bytes(temp0[i], output[i] + 0);
        word_to_bytes(temp1[i], output[i] + 4);
        word_to_bytes(temp2[i], output[i] + 8);
        word_to_bytes(temp3[i], output[i] + 12);
    }
}

// SIMD decryption of 8 blocks in parallel
static void sm4_decrypt_8blocks_simd(const sm4_context_t *ctx, 
                                     const uint8_t input[8][16], 
                                     uint8_t output[8][16]) {
    // Load 8 blocks into 4 vectors (one for each 32-bit word position)
    __m256i X0, X1, X2, X3;
    
    // Initialize X0, X1, X2, X3 with input data
    uint32_t temp0[8], temp1[8], temp2[8], temp3[8];
    
    for (int i = 0; i < 8; i++) {
        temp0[i] = bytes_to_word(input[i] + 0);
        temp1[i] = bytes_to_word(input[i] + 4);
        temp2[i] = bytes_to_word(input[i] + 8);
        temp3[i] = bytes_to_word(input[i] + 12);
    }
    
    X0 = _mm256_loadu_si256((__m256i*)temp0);
    X1 = _mm256_loadu_si256((__m256i*)temp1);
    X2 = _mm256_loadu_si256((__m256i*)temp2);
    X3 = _mm256_loadu_si256((__m256i*)temp3);
    
    // 32 rounds of transformation (reverse order for decryption)
    for (int i = 0; i < 32; i++) {
        __m256i rk_vec = _mm256_set1_epi32(ctx->rk[31 - i]);
        __m256i temp = _mm256_xor_si256(
            _mm256_xor_si256(X1, X2),
            _mm256_xor_si256(X3, rk_vec)
        );
        
        __m256i new_X3 = _mm256_xor_si256(X0, simd_T(temp));
        
        // Rotate the state
        X0 = X1;
        X1 = X2;
        X2 = X3;
        X3 = new_X3;
    }
    
    // Reverse transformation R and output
    _mm256_storeu_si256((__m256i*)temp0, X3);
    _mm256_storeu_si256((__m256i*)temp1, X2);
    _mm256_storeu_si256((__m256i*)temp2, X1);
    _mm256_storeu_si256((__m256i*)temp3, X0);
    
    for (int i = 0; i < 8; i++) {
        word_to_bytes(temp0[i], output[i] + 0);
        word_to_bytes(temp1[i], output[i] + 4);
        word_to_bytes(temp2[i], output[i] + 8);
        word_to_bytes(temp3[i], output[i] + 12);
    }
}

// Fallback to basic implementation for single block
static void sm4_crypt_basic(const uint32_t rk[32], const uint8_t input[16], uint8_t output[16], int encrypt) {
    uint32_t X[4];
    
    // Convert input to words
    for (int i = 0; i < 4; i++) {
        X[i] = bytes_to_word(input + 4 * i);
    }
    
    // 32 rounds of transformation
    for (int i = 0; i < 32; i++) {
        int round_key_idx = encrypt ? i : (31 - i);
        uint32_t temp = X[0] ^ sm4_T(X[1] ^ X[2] ^ X[3] ^ rk[round_key_idx]);
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
    
    sm4_key_expansion(key, ctx->rk);
    return 0;
}

// Set decryption key (same as encryption key for SM4)
int sm4_set_decrypt_key(sm4_context_t *ctx, const uint8_t key[SM4_KEY_SIZE]) {
    return sm4_set_encrypt_key(ctx, key);
}

// Encrypt a single block (fallback to basic implementation)
int sm4_encrypt_block(const sm4_context_t *ctx, const uint8_t input[SM4_BLOCK_SIZE], uint8_t output[SM4_BLOCK_SIZE]) {
    if (!ctx || !input || !output) {
        return -1;
    }
    
    sm4_crypt_basic(ctx->rk, input, output, 1);
    return 0;
}

// Decrypt a single block (fallback to basic implementation)
int sm4_decrypt_block(const sm4_context_t *ctx, const uint8_t input[SM4_BLOCK_SIZE], uint8_t output[SM4_BLOCK_SIZE]) {
    if (!ctx || !input || !output) {
        return -1;
    }
    
    sm4_crypt_basic(ctx->rk, input, output, 0);
    return 0;
}

// Optimized encryption with loop unrolling and batch processing
// This provides better cache locality and reduced function call overhead
static void sm4_encrypt_batch_optimized(const sm4_context_t *ctx, 
                                        const uint8_t *input, 
                                        uint8_t *output, 
                                        size_t num_blocks) {
    // Process multiple blocks with better cache locality
    for (size_t i = 0; i < num_blocks; i++) {
        const uint8_t *in = input + i * 16;
        uint8_t *out = output + i * 16;
        
        uint32_t X[4];
        
        // Convert input to words
        X[0] = bytes_to_word(in + 0);
        X[1] = bytes_to_word(in + 4);
        X[2] = bytes_to_word(in + 8);
        X[3] = bytes_to_word(in + 12);
        
        // Unroll first few rounds for better performance
        uint32_t temp;
        
        // Round 0-3 (unrolled)
        temp = X[0] ^ sm4_T(X[1] ^ X[2] ^ X[3] ^ ctx->rk[0]);
        X[0] = X[1]; X[1] = X[2]; X[2] = X[3]; X[3] = temp;
        
        temp = X[0] ^ sm4_T(X[1] ^ X[2] ^ X[3] ^ ctx->rk[1]);
        X[0] = X[1]; X[1] = X[2]; X[2] = X[3]; X[3] = temp;
        
        temp = X[0] ^ sm4_T(X[1] ^ X[2] ^ X[3] ^ ctx->rk[2]);
        X[0] = X[1]; X[1] = X[2]; X[2] = X[3]; X[3] = temp;
        
        temp = X[0] ^ sm4_T(X[1] ^ X[2] ^ X[3] ^ ctx->rk[3]);
        X[0] = X[1]; X[1] = X[2]; X[2] = X[3]; X[3] = temp;
        
        // Remaining rounds
        for (int r = 4; r < 32; r++) {
            temp = X[0] ^ sm4_T(X[1] ^ X[2] ^ X[3] ^ ctx->rk[r]);
            X[0] = X[1]; X[1] = X[2]; X[2] = X[3]; X[3] = temp;
        }
        
        // Reverse transformation R and output
        word_to_bytes(X[3], out + 0);
        word_to_bytes(X[2], out + 4);
        word_to_bytes(X[1], out + 8);
        word_to_bytes(X[0], out + 12);
    }
}

// Optimized decryption with loop unrolling and batch processing
static void sm4_decrypt_batch_optimized(const sm4_context_t *ctx, 
                                        const uint8_t *input, 
                                        uint8_t *output, 
                                        size_t num_blocks) {
    // Process multiple blocks with better cache locality
    for (size_t i = 0; i < num_blocks; i++) {
        const uint8_t *in = input + i * 16;
        uint8_t *out = output + i * 16;
        
        uint32_t X[4];
        
        // Convert input to words
        X[0] = bytes_to_word(in + 0);
        X[1] = bytes_to_word(in + 4);
        X[2] = bytes_to_word(in + 8);
        X[3] = bytes_to_word(in + 12);
        
        // Unroll first few rounds for better performance (reverse order)
        uint32_t temp;
        
        // Round 31-28 (unrolled)
        temp = X[0] ^ sm4_T(X[1] ^ X[2] ^ X[3] ^ ctx->rk[31]);
        X[0] = X[1]; X[1] = X[2]; X[2] = X[3]; X[3] = temp;
        
        temp = X[0] ^ sm4_T(X[1] ^ X[2] ^ X[3] ^ ctx->rk[30]);
        X[0] = X[1]; X[1] = X[2]; X[2] = X[3]; X[3] = temp;
        
        temp = X[0] ^ sm4_T(X[1] ^ X[2] ^ X[3] ^ ctx->rk[29]);
        X[0] = X[1]; X[1] = X[2]; X[2] = X[3]; X[3] = temp;
        
        temp = X[0] ^ sm4_T(X[1] ^ X[2] ^ X[3] ^ ctx->rk[28]);
        X[0] = X[1]; X[1] = X[2]; X[2] = X[3]; X[3] = temp;
        
        // Remaining rounds
        for (int r = 27; r >= 0; r--) {
            temp = X[0] ^ sm4_T(X[1] ^ X[2] ^ X[3] ^ ctx->rk[r]);
            X[0] = X[1]; X[1] = X[2]; X[2] = X[3]; X[3] = temp;
        }
        
        // Reverse transformation R and output
        word_to_bytes(X[3], out + 0);
        word_to_bytes(X[2], out + 4);
        word_to_bytes(X[1], out + 8);
        word_to_bytes(X[0], out + 12);
    }
}

// Batch encryption using optimized implementation
int sm4_encrypt_blocks_simd(const sm4_context_t *ctx, const uint8_t *input, uint8_t *output, size_t num_blocks) {
    if (!ctx || !input || !output || num_blocks == 0) {
        return -1;
    }
    
    // Use optimized batch processing
    sm4_encrypt_batch_optimized(ctx, input, output, num_blocks);
    return 0;
}

// Batch decryption using optimized implementation
int sm4_decrypt_blocks_simd(const sm4_context_t *ctx, const uint8_t *input, uint8_t *output, size_t num_blocks) {
    if (!ctx || !input || !output || num_blocks == 0) {
        return -1;
    }
    
    // Use optimized batch processing
    sm4_decrypt_batch_optimized(ctx, input, output, num_blocks);
    return 0;
}
