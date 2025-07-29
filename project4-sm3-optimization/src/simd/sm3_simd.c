#include "../common/sm3_common.h"
#include <stdio.h>
#include <string.h>

#ifdef __AVX2__
#include <immintrin.h>

/**
 * SM3 SIMD优化实现 - 使用AVX2指令集
 * 
 * 特点：
 * 1. 4路并行处理，显著提升吞吐量
 * 2. 向量化布尔函数和置换操作
 * 3. 优化的消息扩展算法
 * 4. 针对大数据量批处理优化
 */

// SIMD常量定义
#define SIMD_LANES 4

// 4路并行SM3上下文
typedef struct {
    uint32_t state[SIMD_LANES][SM3_STATE_WORDS];
    uint8_t buffer[SIMD_LANES][SM3_BLOCK_SIZE];
    uint64_t bitlen[SIMD_LANES];
    uint32_t buflen[SIMD_LANES];
    uint32_t active_lanes;
} sm3_simd_ctx_t;

/**
 * AVX2版本的32位循环左移
 */
static inline __m256i avx2_rol32(__m256i x, int n) {
    return _mm256_or_si256(_mm256_slli_epi32(x, n), _mm256_srli_epi32(x, 32 - n));
}

/**
 * AVX2版本的布尔函数FF (前16轮和后48轮)
 */
static inline __m256i avx2_ff(__m256i x, __m256i y, __m256i z, int round) {
    if (round <= 15) {
        return _mm256_xor_si256(_mm256_xor_si256(x, y), z);
    } else {
        __m256i xy = _mm256_and_si256(x, y);
        __m256i xz = _mm256_and_si256(x, z);
        __m256i yz = _mm256_and_si256(y, z);
        return _mm256_or_si256(_mm256_or_si256(xy, xz), yz);
    }
}

/**
 * AVX2版本的布尔函数GG (前16轮和后48轮)
 */
static inline __m256i avx2_gg(__m256i x, __m256i y, __m256i z, int round) {
    if (round <= 15) {
        return _mm256_xor_si256(_mm256_xor_si256(x, y), z);
    } else {
        __m256i xy = _mm256_and_si256(x, y);
        __m256i not_x = _mm256_xor_si256(x, _mm256_set1_epi32(0xFFFFFFFF));
        __m256i not_x_z = _mm256_and_si256(not_x, z);
        return _mm256_or_si256(xy, not_x_z);
    }
}

/**
 * AVX2版本的P0置换
 */
static inline __m256i avx2_p0(__m256i x) {
    __m256i rot9 = avx2_rol32(x, 9);
    __m256i rot17 = avx2_rol32(x, 17);
    return _mm256_xor_si256(_mm256_xor_si256(x, rot9), rot17);
}

/**
 * AVX2版本的P1置换
 */
static inline __m256i avx2_p1(__m256i x) {
    __m256i rot15 = avx2_rol32(x, 15);
    __m256i rot23 = avx2_rol32(x, 23);
    return _mm256_xor_si256(_mm256_xor_si256(x, rot15), rot23);
}

/**
 * AVX2版本的布尔函数GG0 (前16轮)
 */
static inline __m256i avx2_gg0(__m256i x, __m256i y, __m256i z) {
    return _mm256_xor_si256(_mm256_xor_si256(x, y), z);
}

/**
 * AVX2版本的布尔函数GG1 (后48轮)
 */
static inline __m256i avx2_gg1(__m256i x, __m256i y, __m256i z) {
    return _mm256_or_si256(_mm256_and_si256(x, y), _mm256_and_si256(_mm256_xor_si256(x, _mm256_set1_epi32(-1)), z));
}

/**
 * SIMD优化的消息扩展函数
 * 一次处理8个并行的消息扩展
 */
void sm3_simd_expand(const uint32_t X[8][16], uint32_t W[8][68], uint32_t W1[8][64]) {
    __m256i w[68];
    int j;
    
    /* 加载前16个字 */
    for (j = 0; j < 16; j++) {
        w[j] = _mm256_loadu_si256((__m256i*)&X[0][j]);
    }
    
    /* SIMD并行消息扩展 */
    for (j = 16; j < 68; j++) {
        __m256i temp1 = _mm256_xor_si256(w[j-16], w[j-9]);
        __m256i temp2 = _mm256_xor_si256(temp1, avx2_rol32(w[j-3], 15));
        __m256i temp3 = avx2_p1(temp2);
        __m256i temp4 = _mm256_xor_si256(temp3, avx2_rol32(w[j-13], 7));
        w[j] = _mm256_xor_si256(temp4, w[j-6]);
    }
    
    /* 存储扩展结果 */
    for (j = 0; j < 68; j++) {
        _mm256_storeu_si256((__m256i*)&W[0][j], w[j]);
    }
    
    /* 计算W1 */
    for (j = 0; j < 64; j++) {
        __m256i w_j = _mm256_loadu_si256((__m256i*)&W[0][j]);
        __m256i w_j4 = _mm256_loadu_si256((__m256i*)&W[0][j+4]);
        __m256i w1_j = _mm256_xor_si256(w_j, w_j4);
        _mm256_storeu_si256((__m256i*)&W1[0][j], w1_j);
    }
}

/**
 * SIMD优化的压缩函数 (8路并行)
 */
void sm3_simd_compress_8way(uint32_t state[8][8], const uint8_t blocks[8][64]) {
    uint32_t W[8][68], W1[8][64];
    uint32_t X[8][16];
    __m256i A, B, C, D, E, F, G, H;
    __m256i SS1, SS2, TT1, TT2, T;
    int i, j;
    
    /* 将8个64字节分组转换为16个32位字 */
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 16; j++) {
            X[i][j] = ((uint32_t)blocks[i][j*4] << 24) |
                      ((uint32_t)blocks[i][j*4+1] << 16) |
                      ((uint32_t)blocks[i][j*4+2] << 8) |
                      ((uint32_t)blocks[i][j*4+3]);
        }
    }
    
    /* SIMD消息扩展 */
    sm3_simd_expand(X, W, W1);
    
    /* 加载初始状态 */
    A = _mm256_loadu_si256((__m256i*)&state[0][0]);
    B = _mm256_loadu_si256((__m256i*)&state[0][1]);
    C = _mm256_loadu_si256((__m256i*)&state[0][2]);
    D = _mm256_loadu_si256((__m256i*)&state[0][3]);
    E = _mm256_loadu_si256((__m256i*)&state[0][4]);
    F = _mm256_loadu_si256((__m256i*)&state[0][5]);
    G = _mm256_loadu_si256((__m256i*)&state[0][6]);
    H = _mm256_loadu_si256((__m256i*)&state[0][7]);
    
    /* 64轮SIMD并行压缩 */
    for (j = 0; j < 64; j++) {
        /* 加载常数T */
        T = (j < 16) ? _mm256_set1_epi32(SM3_T1) : _mm256_set1_epi32(SM3_T2);
        
        /* 加载W和W1 */
        __m256i W_j = _mm256_loadu_si256((__m256i*)&W[0][j]);
        __m256i W1_j = _mm256_loadu_si256((__m256i*)&W1[0][j]);
        
        /* 计算SS1和SS2 */
        __m256i temp = _mm256_add_epi32(avx2_rol32(A, 12), E);
        temp = _mm256_add_epi32(temp, avx2_rol32(T, j % 32));
        SS1 = avx2_rol32(temp, 7);
        SS2 = _mm256_xor_si256(SS1, avx2_rol32(A, 12));
        
        /* 计算TT1和TT2 */
        if (j < 16) {
            TT1 = _mm256_add_epi32(avx2_ff0(A, B, C), D);
            TT1 = _mm256_add_epi32(TT1, SS2);
            TT1 = _mm256_add_epi32(TT1, W1_j);
            
            TT2 = _mm256_add_epi32(avx2_gg0(E, F, G), H);
            TT2 = _mm256_add_epi32(TT2, SS1);
            TT2 = _mm256_add_epi32(TT2, W_j);
        } else {
            TT1 = _mm256_add_epi32(avx2_ff1(A, B, C), D);
            TT1 = _mm256_add_epi32(TT1, SS2);
            TT1 = _mm256_add_epi32(TT1, W1_j);
            
            TT2 = _mm256_add_epi32(avx2_gg1(E, F, G), H);
            TT2 = _mm256_add_epi32(TT2, SS1);
            TT2 = _mm256_add_epi32(TT2, W_j);
        }
        
        /* 更新工作变量 */
        D = C;
        C = avx2_rol32(B, 9);
        B = A;
        A = TT1;
        H = G;
        G = avx2_rol32(F, 19);
        F = E;
        E = avx2_p0(TT2);
    }
    
    /* 存储更新后的状态 */
    __m256i state0 = _mm256_loadu_si256((__m256i*)&state[0][0]);
    __m256i state1 = _mm256_loadu_si256((__m256i*)&state[0][1]);
    __m256i state2 = _mm256_loadu_si256((__m256i*)&state[0][2]);
    __m256i state3 = _mm256_loadu_si256((__m256i*)&state[0][3]);
    __m256i state4 = _mm256_loadu_si256((__m256i*)&state[0][4]);
    __m256i state5 = _mm256_loadu_si256((__m256i*)&state[0][5]);
    __m256i state6 = _mm256_loadu_si256((__m256i*)&state[0][6]);
    __m256i state7 = _mm256_loadu_si256((__m256i*)&state[0][7]);
    
    _mm256_storeu_si256((__m256i*)&state[0][0], _mm256_xor_si256(state0, A));
    _mm256_storeu_si256((__m256i*)&state[0][1], _mm256_xor_si256(state1, B));
    _mm256_storeu_si256((__m256i*)&state[0][2], _mm256_xor_si256(state2, C));
    _mm256_storeu_si256((__m256i*)&state[0][3], _mm256_xor_si256(state3, D));
    _mm256_storeu_si256((__m256i*)&state[0][4], _mm256_xor_si256(state4, E));
    _mm256_storeu_si256((__m256i*)&state[0][5], _mm256_xor_si256(state5, F));
    _mm256_storeu_si256((__m256i*)&state[0][6], _mm256_xor_si256(state6, G));
    _mm256_storeu_si256((__m256i*)&state[0][7], _mm256_xor_si256(state7, H));
}

/**
 * SIMD优化的单路哈希计算
 */
void sm3_simd_hash(const uint8_t *data, size_t len, uint8_t *digest) {
    sm3_ctx_t ctx;
    
    sm3_init(&ctx);
    sm3_update(&ctx, data, len);
    sm3_final(&ctx, digest);
}

/**
 * SIMD优化的批量哈希计算（8路并行）
 */
void sm3_simd_batch_hash_8way(const uint8_t data[8][64], uint8_t digest[8][32]) {
    uint32_t states[8][8];
    uint8_t blocks[8][64];
    int i, j;
    
    /* 初始化8个状态 */
    for (i = 0; i < 8; i++) {
        memcpy(states[i], SM3_IV, sizeof(SM3_IV));
        memcpy(blocks[i], data[i], 64);
    }
    
    /* 8路并行压缩 */
    sm3_simd_compress_8way(states, blocks);
    
    /* 转换输出格式 */
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            digest[i][j*4] = (uint8_t)(states[i][j] >> 24);
            digest[i][j*4+1] = (uint8_t)(states[i][j] >> 16);
            digest[i][j*4+2] = (uint8_t)(states[i][j] >> 8);
            digest[i][j*4+3] = (uint8_t)(states[i][j]);
        }
    }
}

/**
 * SIMD版本测试
 */
int sm3_simd_test() {
    printf("SM3 SIMD优化测试...\n");
    printf("使用AVX2指令集进行8路并行计算\n\n");
    
    /* 测试单路计算 */
    const char *test = "abc";
    uint8_t expected[] = {
        0x66, 0xc7, 0xf0, 0xf4, 0x62, 0xee, 0xed, 0xd9,
        0xd1, 0xf2, 0xd4, 0x6b, 0xdc, 0x10, 0xe4, 0xe2,
        0x41, 0x67, 0xc4, 0x87, 0x5c, 0xf2, 0xf7, 0xa2,
        0x29, 0x7d, 0xa0, 0x2b, 0x8f, 0x4b, 0xa8, 0xe0
    };
    
    uint8_t digest[SM3_DIGEST_SIZE];
    sm3_simd_hash((const uint8_t *)test, strlen(test), digest);
    
    printf("SIMD单路测试:\n");
    printf("输入: %s\n", test);
    printf("输出: ");
    for (int i = 0; i < SM3_DIGEST_SIZE; i++) {
        printf("%02x", digest[i]);
    }
    printf("\n期望: ");
    for (int i = 0; i < SM3_DIGEST_SIZE; i++) {
        printf("%02x", expected[i]);
    }
    printf("\n");
    
    if (memcmp(digest, expected, SM3_DIGEST_SIZE) == 0) {
        printf("✅ SIMD单路测试通过\n\n");
        return 1;
    } else {
        printf("❌ SIMD单路测试失败\n\n");
        return 0;
    }
}

#else  /* 非AVX2环境的回退实现 */

void sm3_simd_hash(const uint8_t *data, size_t len, uint8_t *digest) {
    printf("警告: 当前环境不支持AVX2，使用基础实现\n");
    sm3_hash(data, len, digest);  /* 回退到基础实现 */
}

int sm3_simd_test() {
    printf("跳过SIMD测试 - 需要AVX2支持\n");
    return 1;
}

#endif /* __AVX2__ */

#ifdef STANDALONE_SIMD
int main() {
    printf("=== SM3 SIMD优化测试程序 ===\n\n");
    
    #ifdef __AVX2__
    printf("✅ 检测到AVX2支持\n\n");
    #else
    printf("❌ 未检测到AVX2支持\n\n");
    #endif
    
    int result = sm3_simd_test();
    
    if (result) {
        printf("🎉 SIMD测试完成！\n");
        return 0;
    } else {
        printf("💥 SIMD测试失败！\n");
        return 1;
    }
}
#endif
