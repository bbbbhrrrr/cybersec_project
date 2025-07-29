/*
 * SM4 GFNI (Galois Field New Instructions) 优化实现
 * 使用 Intel GFNI 指令集进行伽罗瓦域运算优化
 */

#include "sm4_gfni.h"
#include "../common/sm4_common.h"
#include <immintrin.h>
#include <string.h>

// 检查 GFNI 支持
static int check_gfni_support(void) {
    unsigned int eax, ebx, ecx, edx;
    __asm__ volatile("cpuid"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(7), "c"(0));
    return (ecx & (1 << 8)) != 0; // GFNI 支持位
}

// SM4 S-box 的伽罗瓦域矩阵表示
static const uint64_t sm4_sbox_matrix[8] = {
    0x8F1F2F4F8F1F2F4FULL,
    0x0706050403020100ULL,
    0x1716151413121110ULL,
    0x2726252423222120ULL,
    0x3736353433323130ULL,
    0x4746454443424140ULL,
    0x5756555453525150ULL,
    0x6766656463626160ULL
};

// 使用 GFNI 指令优化的 S-box 变换
static inline __m128i sm4_sbox_gfni(__m128i x) {
    // 使用 GF2P8AFFINEQB 指令进行仿射变换
    __m128i matrix = _mm_set_epi64x(sm4_sbox_matrix[1], sm4_sbox_matrix[0]);
    __m128i result = _mm_gf2p8affine_epi64_epi8(x, matrix, 0x63);
    
    // 进一步的伽罗瓦域变换
    matrix = _mm_set_epi64x(sm4_sbox_matrix[3], sm4_sbox_matrix[2]);
    result = _mm_gf2p8affine_epi64_epi8(result, matrix, 0x00);
    
    return result;
}

// 使用 GFNI 优化的伽罗瓦域乘法
static inline __m128i gf_multiply_gfni(__m128i a, __m128i b) {
    return _mm_gf2p8mul_epi8(a, b);
}

// GFNI 优化的轮函数
static inline __m128i sm4_round_gfni(__m128i state, uint32_t rk) {
    // 将轮密钥广播到128位
    __m128i round_key = _mm_set1_epi32(rk);
    
    // XOR 轮密钥
    __m128i temp = _mm_xor_si128(state, round_key);
    
    // GFNI 优化的 S-box 变换
    temp = sm4_sbox_gfni(temp);
    
    // 线性变换 L，使用 GFNI 指令优化
    __m128i l_matrix = _mm_set_epi64x(0x0102040810204080ULL, 0x8040201008040201ULL);
    temp = _mm_gf2p8affine_epi64_epi8(temp, l_matrix, 0x00);
    
    // 旋转操作
    __m128i rotated2 = _mm_or_si128(_mm_slli_epi32(temp, 2), _mm_srli_epi32(temp, 30));
    __m128i rotated10 = _mm_or_si128(_mm_slli_epi32(temp, 10), _mm_srli_epi32(temp, 22));
    __m128i rotated18 = _mm_or_si128(_mm_slli_epi32(temp, 18), _mm_srli_epi32(temp, 14));
    __m128i rotated24 = _mm_or_si128(_mm_slli_epi32(temp, 24), _mm_srli_epi32(temp, 8));
    
    temp = _mm_xor_si128(temp, rotated2);
    temp = _mm_xor_si128(temp, rotated10);
    temp = _mm_xor_si128(temp, rotated18);
    temp = _mm_xor_si128(temp, rotated24);
    
    return temp;
}

int sm4_gfni_encrypt_block(const uint8_t *plaintext, uint8_t *ciphertext, const uint32_t *round_keys) {
    if (!check_gfni_support()) {
        return -1; // GFNI 不支持
    }
    
    // 加载明文
    __m128i state = _mm_loadu_si128((__m128i*)plaintext);
    
    // 字节序转换
    state = _mm_shuffle_epi8(state, _mm_set_epi8(
        12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3
    ));
    
    // 32 轮加密
    for (int i = 0; i < 32; i++) {
        state = sm4_round_gfni(state, round_keys[i]);
    }
    
    // 反变换
    state = _mm_shuffle_epi8(state, _mm_set_epi8(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    ));
    
    // 存储密文
    _mm_storeu_si128((__m128i*)ciphertext, state);
    
    return 0;
}

int sm4_gfni_decrypt_block(const uint8_t *ciphertext, uint8_t *plaintext, const uint32_t *round_keys) {
    if (!check_gfni_support()) {
        return -1;
    }
    
    // 加载密文
    __m128i state = _mm_loadu_si128((__m128i*)ciphertext);
    
    // 字节序转换
    state = _mm_shuffle_epi8(state, _mm_set_epi8(
        12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3
    ));
    
    // 32 轮解密（逆序轮密钥）
    for (int i = 31; i >= 0; i--) {
        state = sm4_round_gfni(state, round_keys[i]);
    }
    
    // 反变换
    state = _mm_shuffle_epi8(state, _mm_set_epi8(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    ));
    
    // 存储明文
    _mm_storeu_si128((__m128i*)plaintext, state);
    
    return 0;
}

// 使用 VPTERNLOGD 指令优化的8块并行加密
int sm4_gfni_encrypt_8blocks_avx512(const uint8_t *plaintext, uint8_t *ciphertext, const uint32_t *round_keys) {
    if (!check_gfni_support()) {
        return -1;
    }
    
    // 加载8个块到 ZMM 寄存器
    __m512i state = _mm512_loadu_si512((__m512i*)plaintext);
    
    // 字节序转换
    __m512i shuffle_mask = _mm512_set_epi8(
        60, 61, 62, 63, 56, 57, 58, 59, 52, 53, 54, 55, 48, 49, 50, 51,
        44, 45, 46, 47, 40, 41, 42, 43, 36, 37, 38, 39, 32, 33, 34, 35,
        28, 29, 30, 31, 24, 25, 26, 27, 20, 21, 22, 23, 16, 17, 18, 19,
        12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3
    );
    
    state = _mm512_shuffle_epi8(state, shuffle_mask);
    
    // 32 轮并行加密
    for (int i = 0; i < 32; i++) {
        __m512i round_key = _mm512_set1_epi32(round_keys[i]);
        
        // XOR 轮密钥
        state = _mm512_xor_si512(state, round_key);
        
        // GFNI S-box 变换（分成4个128位块处理）
        __m128i block0 = _mm512_extracti128_epi32(state, 0);
        __m128i block1 = _mm512_extracti128_epi32(state, 1);
        __m128i block2 = _mm512_extracti128_epi32(state, 2);
        __m128i block3 = _mm512_extracti128_epi32(state, 3);
        
        block0 = sm4_sbox_gfni(block0);
        block1 = sm4_sbox_gfni(block1);
        block2 = sm4_sbox_gfni(block2);
        block3 = sm4_sbox_gfni(block3);
        
        state = _mm512_inserti128_epi32(state, block0, 0);
        state = _mm512_inserti128_epi32(state, block1, 1);
        state = _mm512_inserti128_epi32(state, block2, 2);
        state = _mm512_inserti128_epi32(state, block3, 3);
        
        // 使用 VPTERNLOGD 进行线性变换优化
        __m512i temp = state;
        __m512i rotated2 = _mm512_or_si512(_mm512_slli_epi32(temp, 2), _mm512_srli_epi32(temp, 30));
        __m512i rotated10 = _mm512_or_si512(_mm512_slli_epi32(temp, 10), _mm512_srli_epi32(temp, 22));
        __m512i rotated18 = _mm512_or_si512(_mm512_slli_epi32(temp, 18), _mm512_srli_epi32(temp, 14));
        
        // 使用三元逻辑指令 VPTERNLOGD 进行复合 XOR 操作
        state = _mm512_ternarylogic_epi32(temp, rotated2, rotated10, 0x96); // A XOR B XOR C
        state = _mm512_xor_si512(state, rotated18);
        
        __m512i rotated24 = _mm512_or_si512(_mm512_slli_epi32(temp, 24), _mm512_srli_epi32(temp, 8));
        state = _mm512_xor_si512(state, rotated24);
    }
    
    // 反变换
    __m512i reverse_mask = _mm512_set_epi8(
        48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
        32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    );
    
    state = _mm512_shuffle_epi8(state, reverse_mask);
    
    // 存储密文
    _mm512_storeu_si512((__m512i*)ciphertext, state);
    
    return 0;
}

// ECB 模式加密（GFNI优化）
int sm4_gfni_encrypt_ecb(const uint8_t *plaintext, uint8_t *ciphertext, size_t length, const uint32_t *round_keys) {
    if (!check_gfni_support()) {
        return -1;
    }
    
    if (length % 16 != 0) {
        return -2;
    }
    
    size_t blocks = length / 16;
    size_t i = 0;
    
    // 8块并行处理（AVX-512）
    for (; i + 8 <= blocks; i += 8) {
        sm4_gfni_encrypt_8blocks_avx512(plaintext + i * 16, ciphertext + i * 16, round_keys);
    }
    
    // 处理剩余块
    for (; i < blocks; i++) {
        sm4_gfni_encrypt_block(plaintext + i * 16, ciphertext + i * 16, round_keys);
    }
    
    return 0;
}

// ECB 模式解密（GFNI优化）
int sm4_gfni_decrypt_ecb(const uint8_t *ciphertext, uint8_t *plaintext, size_t length, const uint32_t *round_keys) {
    if (!check_gfni_support()) {
        return -1;
    }
    
    if (length % 16 != 0) {
        return -2;
    }
    
    size_t blocks = length / 16;
    
    for (size_t i = 0; i < blocks; i++) {
        sm4_gfni_decrypt_block(ciphertext + i * 16, plaintext + i * 16, round_keys);
    }
    
    return 0;
}
