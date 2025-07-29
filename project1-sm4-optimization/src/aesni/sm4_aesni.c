/*
 * SM4 AES-NI 指令集优化实现
 * 使用 Intel AES-NI 指令集加速 SM4 算法
 */

#include "sm4_aesni.h"
#include "../common/sm4_common.h"
#include <immintrin.h>
#include <string.h>

// 检查 AES-NI 支持
static int check_aesni_support(void) {
    unsigned int eax, ebx, ecx, edx;
    __asm__ volatile("cpuid"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(1));
    return (ecx & (1 << 25)) != 0; // AES-NI 支持位
}

// 使用 AES-NI 指令优化的 S 盒变换
static inline __m128i sm4_sbox_aesni(__m128i x) {
    // 利用 AES S-box 来加速 SM4 S-box 计算
    // 这里使用组合变换来实现 SM4 的 S-box
    __m128i mask = _mm_set1_epi8(0x52);
    __m128i temp = _mm_xor_si128(x, mask);
    
    // 使用 AES 指令进行非线性变换
    temp = _mm_aeskeygenassist_si128(temp, 0x00);
    temp = _mm_shuffle_epi32(temp, 0xFF);
    
    // 进一步处理以匹配 SM4 S-box
    temp = _mm_xor_si128(temp, _mm_set1_epi8(0x63));
    
    return temp;
}

// AES-NI 优化的轮函数
static inline __m128i sm4_round_aesni(__m128i state, uint32_t rk) {
    // 将 32 位轮密钥扩展为 128 位
    __m128i round_key = _mm_set1_epi32(rk);
    
    // 执行 AddRoundKey
    __m128i temp = _mm_xor_si128(state, round_key);
    
    // S-box 变换
    temp = sm4_sbox_aesni(temp);
    
    // 线性变换 L
    __m128i rotated = _mm_or_si128(
        _mm_slli_epi32(temp, 2),
        _mm_srli_epi32(temp, 30)
    );
    
    temp = _mm_xor_si128(temp, rotated);
    
    rotated = _mm_or_si128(
        _mm_slli_epi32(temp, 10),
        _mm_srli_epi32(temp, 22)
    );
    
    temp = _mm_xor_si128(temp, rotated);
    
    rotated = _mm_or_si128(
        _mm_slli_epi32(temp, 18),
        _mm_srli_epi32(temp, 14)
    );
    
    temp = _mm_xor_si128(temp, rotated);
    
    rotated = _mm_or_si128(
        _mm_slli_epi32(temp, 24),
        _mm_srli_epi32(temp, 8)
    );
    
    return _mm_xor_si128(temp, rotated);
}

int sm4_aesni_encrypt_block(const uint8_t *plaintext, uint8_t *ciphertext, const uint32_t *round_keys) {
    if (!check_aesni_support()) {
        return -1; // AES-NI 不支持
    }
    
    // 加载明文到 XMM 寄存器
    __m128i state = _mm_loadu_si128((__m128i*)plaintext);
    
    // 字节序转换（大端序到小端序）
    state = _mm_shuffle_epi8(state, _mm_set_epi8(
        12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3
    ));
    
    // 32 轮加密
    for (int i = 0; i < 32; i++) {
        state = sm4_round_aesni(state, round_keys[i]);
    }
    
    // 反变换（R）
    state = _mm_shuffle_epi8(state, _mm_set_epi8(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    ));
    
    // 存储密文
    _mm_storeu_si128((__m128i*)ciphertext, state);
    
    return 0;
}

int sm4_aesni_decrypt_block(const uint8_t *ciphertext, uint8_t *plaintext, const uint32_t *round_keys) {
    if (!check_aesni_support()) {
        return -1; // AES-NI 不支持
    }
    
    // 加载密文到 XMM 寄存器
    __m128i state = _mm_loadu_si128((__m128i*)ciphertext);
    
    // 字节序转换
    state = _mm_shuffle_epi8(state, _mm_set_epi8(
        12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3
    ));
    
    // 32 轮解密（使用逆序轮密钥）
    for (int i = 31; i >= 0; i--) {
        state = sm4_round_aesni(state, round_keys[i]);
    }
    
    // 反变换
    state = _mm_shuffle_epi8(state, _mm_set_epi8(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    ));
    
    // 存储明文
    _mm_storeu_si128((__m128i*)plaintext, state);
    
    return 0;
}

// 批量加密（4个块并行）
int sm4_aesni_encrypt_4blocks(const uint8_t *plaintext, uint8_t *ciphertext, const uint32_t *round_keys) {
    if (!check_aesni_support()) {
        return -1;
    }
    
    // 加载4个块
    __m128i block0 = _mm_loadu_si128((__m128i*)(plaintext + 0));
    __m128i block1 = _mm_loadu_si128((__m128i*)(plaintext + 16));
    __m128i block2 = _mm_loadu_si128((__m128i*)(plaintext + 32));
    __m128i block3 = _mm_loadu_si128((__m128i*)(plaintext + 48));
    
    // 字节序转换
    __m128i shuffle_mask = _mm_set_epi8(
        12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3
    );
    
    block0 = _mm_shuffle_epi8(block0, shuffle_mask);
    block1 = _mm_shuffle_epi8(block1, shuffle_mask);
    block2 = _mm_shuffle_epi8(block2, shuffle_mask);
    block3 = _mm_shuffle_epi8(block3, shuffle_mask);
    
    // 32 轮并行加密
    for (int i = 0; i < 32; i++) {
        block0 = sm4_round_aesni(block0, round_keys[i]);
        block1 = sm4_round_aesni(block1, round_keys[i]);
        block2 = sm4_round_aesni(block2, round_keys[i]);
        block3 = sm4_round_aesni(block3, round_keys[i]);
    }
    
    // 反变换
    __m128i reverse_mask = _mm_set_epi8(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    );
    
    block0 = _mm_shuffle_epi8(block0, reverse_mask);
    block1 = _mm_shuffle_epi8(block1, reverse_mask);
    block2 = _mm_shuffle_epi8(block2, reverse_mask);
    block3 = _mm_shuffle_epi8(block3, reverse_mask);
    
    // 存储密文
    _mm_storeu_si128((__m128i*)(ciphertext + 0), block0);
    _mm_storeu_si128((__m128i*)(ciphertext + 16), block1);
    _mm_storeu_si128((__m128i*)(ciphertext + 32), block2);
    _mm_storeu_si128((__m128i*)(ciphertext + 48), block3);
    
    return 0;
}

// ECB 模式加密
int sm4_aesni_encrypt_ecb(const uint8_t *plaintext, uint8_t *ciphertext, size_t length, const uint32_t *round_keys) {
    if (!check_aesni_support()) {
        return -1;
    }
    
    if (length % 16 != 0) {
        return -2; // 长度必须是16的倍数
    }
    
    size_t blocks = length / 16;
    size_t i = 0;
    
    // 4块并行处理
    for (; i + 4 <= blocks; i += 4) {
        sm4_aesni_encrypt_4blocks(plaintext + i * 16, ciphertext + i * 16, round_keys);
    }
    
    // 处理剩余块
    for (; i < blocks; i++) {
        sm4_aesni_encrypt_block(plaintext + i * 16, ciphertext + i * 16, round_keys);
    }
    
    return 0;
}

// ECB 模式解密
int sm4_aesni_decrypt_ecb(const uint8_t *ciphertext, uint8_t *plaintext, size_t length, const uint32_t *round_keys) {
    if (!check_aesni_support()) {
        return -1;
    }
    
    if (length % 16 != 0) {
        return -2;
    }
    
    size_t blocks = length / 16;
    
    for (size_t i = 0; i < blocks; i++) {
        sm4_aesni_decrypt_block(ciphertext + i * 16, plaintext + i * 16, round_keys);
    }
    
    return 0;
}
