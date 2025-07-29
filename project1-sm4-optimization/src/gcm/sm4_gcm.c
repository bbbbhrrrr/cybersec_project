/*
 * SM4-GCM (Galois/Counter Mode) 认证加密模式实现
 * 基于SM4算法实现GCM工作模式，提供加密和认证功能
 */

#include "sm4_gcm.h"
#include "../common/sm4_common.h"
#include "../simd/sm4_simd.h"
#include <string.h>
#include <immintrin.h>

// GCM 模式常量
#define GCM_BLOCK_SIZE 16
#define GCM_IV_SIZE 12
#define GCM_TAG_SIZE 16

// GF(2^128) 多项式约简
static const uint64_t GCM_POLYNOMIAL = 0xE100000000000000ULL;

// 预计算的乘法表
static uint64_t gcm_mult_table[256][2];
static int gcm_table_initialized = 0;

// 初始化GCM乘法表
static void init_gcm_table(const uint8_t *H) {
    if (gcm_table_initialized) return;
    
    uint64_t h_high = 0, h_low = 0;
    
    // 将H转换为64位数字
    for (int i = 0; i < 8; i++) {
        h_high = (h_high << 8) | H[i];
        h_low = (h_low << 8) | H[i + 8];
    }
    
    // 生成乘法表
    gcm_mult_table[0][0] = 0;
    gcm_mult_table[0][1] = 0;
    gcm_mult_table[128][0] = h_high;
    gcm_mult_table[128][1] = h_low;
    
    for (int i = 64; i > 0; i >>= 1) {
        uint64_t tmp_high = gcm_mult_table[i << 1][0];
        uint64_t tmp_low = gcm_mult_table[i << 1][1];
        
        // 右移1位
        gcm_mult_table[i][1] = (tmp_low >> 1) | (tmp_high << 63);
        gcm_mult_table[i][0] = tmp_high >> 1;
        
        // 如果最低位为1，则异或约简多项式
        if (tmp_low & 1) {
            gcm_mult_table[i][1] ^= GCM_POLYNOMIAL;
        }
    }
    
    // 生成组合表
    for (int i = 2; i < 256; i <<= 1) {
        for (int j = 1; j < i; j++) {
            gcm_mult_table[i + j][0] = gcm_mult_table[i][0] ^ gcm_mult_table[j][0];
            gcm_mult_table[i + j][1] = gcm_mult_table[i][1] ^ gcm_mult_table[j][1];
        }
    }
    
    gcm_table_initialized = 1;
}

// GF(2^128) 乘法（优化版本）
static void gcm_mult(uint8_t *result, const uint8_t *x, const uint8_t *y) {
    uint64_t z_high = 0, z_low = 0;
    uint64_t v_high = 0, v_low = 0;
    
    // 将y转换为64位数字
    for (int i = 0; i < 8; i++) {
        v_high = (v_high << 8) | y[i];
        v_low = (v_low << 8) | y[i + 8];
    }
    
    // 逐字节乘法
    for (int i = 0; i < 16; i++) {
        uint8_t byte = x[i];
        for (int j = 0; j < 8; j++) {
            if (byte & (1 << (7 - j))) {
                z_high ^= v_high;
                z_low ^= v_low;
            }
            
            // v右移1位
            uint64_t carry = v_high & 1;
            v_high >>= 1;
            v_low = (v_low >> 1) | (carry << 63);
            
            // 如果原最低位为1，异或约简多项式
            if (v_low & 0x8000000000000000ULL) {
                v_high ^= 0xE100000000000000ULL;
            }
            v_low &= 0x7FFFFFFFFFFFFFFFULL;
        }
    }
    
    // 将结果转换回字节数组
    for (int i = 0; i < 8; i++) {
        result[i] = (z_high >> (56 - 8 * i)) & 0xFF;
        result[i + 8] = (z_low >> (56 - 8 * i)) & 0xFF;
    }
}

// SIMD优化的GCM乘法
static void gcm_mult_simd(uint8_t *result, const uint8_t *x, const uint8_t *y) {
    __m128i a = _mm_loadu_si128((__m128i*)x);
    __m128i b = _mm_loadu_si128((__m128i*)y);
    
    // 使用PCLMULQDQ指令进行多项式乘法
    __m128i tmp1 = _mm_clmulepi64_si128(a, b, 0x00);  // low * low
    __m128i tmp2 = _mm_clmulepi64_si128(a, b, 0x11);  // high * high
    __m128i tmp3 = _mm_clmulepi64_si128(a, b, 0x01);  // low * high
    __m128i tmp4 = _mm_clmulepi64_si128(a, b, 0x10);  // high * low
    
    // 合并中间项
    __m128i tmp = _mm_xor_si128(tmp3, tmp4);
    __m128i tmp_high = _mm_unpackhi_epi64(_mm_setzero_si128(), tmp);
    __m128i tmp_low = _mm_unpacklo_epi64(tmp, _mm_setzero_si128());
    
    tmp1 = _mm_xor_si128(tmp1, tmp_low);
    tmp2 = _mm_xor_si128(tmp2, tmp_high);
    
    // GF(2^128) 约简
    __m128i poly = _mm_set_epi32(0xE1000000, 0x00000000, 0x00000000, 0x00000000);
    
    // 第一次约简
    __m128i tmp_a = _mm_clmulepi64_si128(tmp2, poly, 0x01);
    tmp1 = _mm_xor_si128(tmp1, _mm_slli_si128(tmp_a, 8));
    tmp2 = _mm_xor_si128(tmp2, _mm_srli_si128(tmp_a, 8));
    
    // 第二次约简
    tmp_a = _mm_clmulepi64_si128(tmp2, poly, 0x00);
    tmp1 = _mm_xor_si128(tmp1, tmp_a);
    
    _mm_storeu_si128((__m128i*)result, tmp1);
}

// GHASH 函数
static void ghash(uint8_t *output, const uint8_t *h, const uint8_t *input, size_t length) {
    uint8_t y[GCM_BLOCK_SIZE] = {0};
    
    size_t blocks = length / GCM_BLOCK_SIZE;
    for (size_t i = 0; i < blocks; i++) {
        // Y XOR X_i
        for (int j = 0; j < GCM_BLOCK_SIZE; j++) {
            y[j] ^= input[i * GCM_BLOCK_SIZE + j];
        }
        
        // Y = Y * H
        gcm_mult_simd(y, y, h);
    }
    
    // 处理剩余字节
    size_t remaining = length % GCM_BLOCK_SIZE;
    if (remaining > 0) {
        uint8_t last_block[GCM_BLOCK_SIZE] = {0};
        memcpy(last_block, input + blocks * GCM_BLOCK_SIZE, remaining);
        
        for (int j = 0; j < GCM_BLOCK_SIZE; j++) {
            y[j] ^= last_block[j];
        }
        
        gcm_mult_simd(y, y, h);
    }
    
    memcpy(output, y, GCM_BLOCK_SIZE);
}

// 初始化GCM上下文
int sm4_gcm_init(sm4_gcm_context *ctx, const uint8_t *key) {
    if (!ctx || !key) {
        return -1;
    }
    
    memset(ctx, 0, sizeof(sm4_gcm_context));
    
    // 生成SM4轮密钥
    if (sm4_key_schedule(key, ctx->round_keys) != 0) {
        return -2;
    }
    
    // 计算H = E_K(0^128)
    uint8_t zero_block[GCM_BLOCK_SIZE] = {0};
    sm4_encrypt_block(zero_block, ctx->h, ctx->round_keys);
    
    // 初始化乘法表
    init_gcm_table(ctx->h);
    
    ctx->initialized = 1;
    return 0;
}

// 开始GCM加密/解密
int sm4_gcm_start(sm4_gcm_context *ctx, const uint8_t *iv, size_t iv_len, 
                  const uint8_t *aad, size_t aad_len) {
    if (!ctx || !ctx->initialized || !iv) {
        return -1;
    }
    
    memset(ctx->j0, 0, GCM_BLOCK_SIZE);
    
    // 计算初始计数器J0
    if (iv_len == GCM_IV_SIZE) {
        // 标准96位IV
        memcpy(ctx->j0, iv, GCM_IV_SIZE);
        ctx->j0[15] = 1;
    } else {
        // 非标准长度IV
        ghash(ctx->j0, ctx->h, iv, iv_len);
        
        // 添加长度信息
        uint8_t len_block[GCM_BLOCK_SIZE] = {0};
        uint64_t iv_bits = iv_len * 8;
        for (int i = 0; i < 8; i++) {
            len_block[15 - i] = (iv_bits >> (8 * i)) & 0xFF;
        }
        
        for (int i = 0; i < GCM_BLOCK_SIZE; i++) {
            ctx->j0[i] ^= len_block[i];
        }
        
        gcm_mult_simd(ctx->j0, ctx->j0, ctx->h);
    }
    
    // 计算初始GHASH值
    memset(ctx->ghash_state, 0, GCM_BLOCK_SIZE);
    if (aad && aad_len > 0) {
        ghash(ctx->ghash_state, ctx->h, aad, aad_len);
        ctx->aad_len = aad_len;
    }
    
    // 初始化计数器
    memcpy(ctx->counter, ctx->j0, GCM_BLOCK_SIZE);
    ctx->counter_len = 0;
    
    return 0;
}

// 增加计数器
static void increment_counter(uint8_t *counter) {
    for (int i = 15; i >= 12; i--) {
        if (++counter[i] != 0) {
            break;
        }
    }
}

// GCM加密更新
int sm4_gcm_encrypt_update(sm4_gcm_context *ctx, const uint8_t *plaintext, 
                          uint8_t *ciphertext, size_t length) {
    if (!ctx || !ctx->initialized) {
        return -1;
    }
    
    size_t remaining = length;
    size_t offset = 0;
    
    while (remaining > 0) {
        // 生成密钥流
        increment_counter(ctx->counter);
        uint8_t keystream[GCM_BLOCK_SIZE];
        sm4_encrypt_block(ctx->counter, keystream, ctx->round_keys);
        
        size_t block_size = (remaining < GCM_BLOCK_SIZE) ? remaining : GCM_BLOCK_SIZE;
        
        // 加密
        for (size_t i = 0; i < block_size; i++) {
            ciphertext[offset + i] = plaintext[offset + i] ^ keystream[i];
        }
        
        // 更新GHASH
        uint8_t cipher_block[GCM_BLOCK_SIZE] = {0};
        memcpy(cipher_block, ciphertext + offset, block_size);
        
        for (int i = 0; i < GCM_BLOCK_SIZE; i++) {
            ctx->ghash_state[i] ^= cipher_block[i];
        }
        gcm_mult_simd(ctx->ghash_state, ctx->ghash_state, ctx->h);
        
        offset += block_size;
        remaining -= block_size;
    }
    
    ctx->counter_len += length;
    return 0;
}

// GCM解密更新
int sm4_gcm_decrypt_update(sm4_gcm_context *ctx, const uint8_t *ciphertext, 
                          uint8_t *plaintext, size_t length) {
    if (!ctx || !ctx->initialized) {
        return -1;
    }
    
    size_t remaining = length;
    size_t offset = 0;
    
    while (remaining > 0) {
        // 生成密钥流
        increment_counter(ctx->counter);
        uint8_t keystream[GCM_BLOCK_SIZE];
        sm4_encrypt_block(ctx->counter, keystream, ctx->round_keys);
        
        size_t block_size = (remaining < GCM_BLOCK_SIZE) ? remaining : GCM_BLOCK_SIZE;
        
        // 更新GHASH（解密前）
        uint8_t cipher_block[GCM_BLOCK_SIZE] = {0};
        memcpy(cipher_block, ciphertext + offset, block_size);
        
        for (int i = 0; i < GCM_BLOCK_SIZE; i++) {
            ctx->ghash_state[i] ^= cipher_block[i];
        }
        gcm_mult_simd(ctx->ghash_state, ctx->ghash_state, ctx->h);
        
        // 解密
        for (size_t i = 0; i < block_size; i++) {
            plaintext[offset + i] = ciphertext[offset + i] ^ keystream[i];
        }
        
        offset += block_size;
        remaining -= block_size;
    }
    
    ctx->counter_len += length;
    return 0;
}

// 完成GCM操作并生成标签
int sm4_gcm_finish(sm4_gcm_context *ctx, uint8_t *tag) {
    if (!ctx || !ctx->initialized || !tag) {
        return -1;
    }
    
    // 添加长度信息到GHASH
    uint8_t len_block[GCM_BLOCK_SIZE] = {0};
    uint64_t aad_bits = ctx->aad_len * 8;
    uint64_t ct_bits = ctx->counter_len * 8;
    
    // AAD长度（高64位）
    for (int i = 0; i < 8; i++) {
        len_block[7 - i] = (aad_bits >> (8 * i)) & 0xFF;
    }
    
    // 密文长度（低64位）
    for (int i = 0; i < 8; i++) {
        len_block[15 - i] = (ct_bits >> (8 * i)) & 0xFF;
    }
    
    // 最终GHASH计算
    for (int i = 0; i < GCM_BLOCK_SIZE; i++) {
        ctx->ghash_state[i] ^= len_block[i];
    }
    gcm_mult_simd(ctx->ghash_state, ctx->ghash_state, ctx->h);
    
    // 生成认证标签
    uint8_t tag_mask[GCM_BLOCK_SIZE];
    sm4_encrypt_block(ctx->j0, tag_mask, ctx->round_keys);
    
    for (int i = 0; i < GCM_TAG_SIZE; i++) {
        tag[i] = ctx->ghash_state[i] ^ tag_mask[i];
    }
    
    return 0;
}

// 验证标签
int sm4_gcm_verify_tag(const uint8_t *tag1, const uint8_t *tag2) {
    if (!tag1 || !tag2) {
        return -1;
    }
    
    uint8_t diff = 0;
    for (int i = 0; i < GCM_TAG_SIZE; i++) {
        diff |= tag1[i] ^ tag2[i];
    }
    
    return (diff == 0) ? 0 : -1;
}

// 一次性加密函数
int sm4_gcm_encrypt(const uint8_t *key, const uint8_t *iv, size_t iv_len,
                   const uint8_t *aad, size_t aad_len,
                   const uint8_t *plaintext, uint8_t *ciphertext, size_t length,
                   uint8_t *tag) {
    sm4_gcm_context ctx;
    
    if (sm4_gcm_init(&ctx, key) != 0) {
        return -1;
    }
    
    if (sm4_gcm_start(&ctx, iv, iv_len, aad, aad_len) != 0) {
        return -2;
    }
    
    if (sm4_gcm_encrypt_update(&ctx, plaintext, ciphertext, length) != 0) {
        return -3;
    }
    
    if (sm4_gcm_finish(&ctx, tag) != 0) {
        return -4;
    }
    
    return 0;
}

// 一次性解密函数
int sm4_gcm_decrypt(const uint8_t *key, const uint8_t *iv, size_t iv_len,
                   const uint8_t *aad, size_t aad_len,
                   const uint8_t *ciphertext, uint8_t *plaintext, size_t length,
                   const uint8_t *tag) {
    sm4_gcm_context ctx;
    uint8_t computed_tag[GCM_TAG_SIZE];
    
    if (sm4_gcm_init(&ctx, key) != 0) {
        return -1;
    }
    
    if (sm4_gcm_start(&ctx, iv, iv_len, aad, aad_len) != 0) {
        return -2;
    }
    
    if (sm4_gcm_decrypt_update(&ctx, ciphertext, plaintext, length) != 0) {
        return -3;
    }
    
    if (sm4_gcm_finish(&ctx, computed_tag) != 0) {
        return -4;
    }
    
    if (sm4_gcm_verify_tag(tag, computed_tag) != 0) {
        // 标签验证失败，清除明文
        memset(plaintext, 0, length);
        return -5;
    }
    
    return 0;
}
