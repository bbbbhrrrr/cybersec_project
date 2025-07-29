#include "../common/sm3_common.h"
#include <stdio.h>
#include <string.h>

/**
 * SM3基础实现 - 标准版本
 * 严格按照GM/T 0004-2012标准实现
 * 
 * 本实现为未优化的基础版本，便于理解算法原理
 * 性能优化版本见其他模块
 */

// SM3算法常量
#define T1 0x79CC4519
#define T2 0x7A879D8A

// 循环左移宏
#define ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

// SM3布尔函数 FF
static uint32_t ff(uint32_t x, uint32_t y, uint32_t z, int j) {
    if (j >= 0 && j <= 15) {
        return x ^ y ^ z;
    } else {
        return (x & y) | (x & z) | (y & z);
    }
}

// SM3布尔函数 GG  
static uint32_t gg(uint32_t x, uint32_t y, uint32_t z, int j) {
    if (j >= 0 && j <= 15) {
        return x ^ y ^ z;
    } else {
        return (x & y) | (~x & z);
    }
}

// P0置换
static uint32_t p0(uint32_t x) {
    return x ^ ROTL(x, 9) ^ ROTL(x, 17);
}

// P1置换
static uint32_t p1(uint32_t x) {
    return x ^ ROTL(x, 15) ^ ROTL(x, 23);
}

// 字节序转换 - 大端序读取32位整数
static uint32_t get_uint32_be(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | 
           ((uint32_t)p[1] << 16) | 
           ((uint32_t)p[2] << 8)  | 
           ((uint32_t)p[3]);
}

// 字节序转换 - 大端序写入32位整数
static void put_uint32_be(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)(v);
}

// 字节序转换 - 大端序写入64位整数
static void put_uint64_be(uint8_t *p, uint64_t v) {
    put_uint32_be(p, (uint32_t)(v >> 32));
    put_uint32_be(p + 4, (uint32_t)v);
}

/**
 * SM3压缩函数
 * 这是SM3算法的核心，处理一个512位数据块
 */
static void sm3_compress_basic(uint32_t digest[SM3_STATE_WORDS], const uint8_t block[SM3_BLOCK_SIZE]) {
    uint32_t w[68], w1[64];
    uint32_t A, B, C, D, E, F, G, H;
    uint32_t ss1, ss2, tt1, tt2;
    int j;
    
    // 消息扩展 - 前16个字直接从输入块获取
    for (j = 0; j < 16; j++) {
        w[j] = get_uint32_be(block + j * 4);
    }
    
    // 消息扩展 - 扩展到68个字
    for (j = 16; j < 68; j++) {
        w[j] = p1(w[j-16] ^ w[j-9] ^ ROTL(w[j-3], 15)) ^ ROTL(w[j-13], 7) ^ w[j-6];
    }
    
    // 计算W'
    for (j = 0; j < 64; j++) {
        w1[j] = w[j] ^ w[j + 4];
    }
    
    // 初始化工作变量
    A = digest[0]; B = digest[1]; C = digest[2]; D = digest[3];
    E = digest[4]; F = digest[5]; G = digest[6]; H = digest[7];
    
    // 64轮压缩函数迭代
    for (j = 0; j < 64; j++) {
        uint32_t T = (j >= 0 && j <= 15) ? T1 : T2;
        
        ss1 = ROTL(ROTL(A, 12) + E + ROTL(T, j % 32), 7);
        ss2 = ss1 ^ ROTL(A, 12);
        
        tt1 = ff(A, B, C, j) + D + ss2 + w1[j];
        tt2 = gg(E, F, G, j) + H + ss1 + w[j];
        
        D = C;
        C = ROTL(B, 9);
        B = A;
        A = tt1;
        H = G;
        G = ROTL(F, 19);
        F = E;
        E = p0(tt2);
    }
    
    // 输出反馈
    digest[0] ^= A; digest[1] ^= B; digest[2] ^= C; digest[3] ^= D;
    digest[4] ^= E; digest[5] ^= F; digest[6] ^= G; digest[7] ^= H;
}

/**
 * 初始化SM3上下文
 */
void sm3_init(sm3_ctx_t *ctx) {
    // 设置初始值向量IV
    ctx->state[0] = 0x7380166F;
    ctx->state[1] = 0x4914B2B9;
    ctx->state[2] = 0x172442D7;
    ctx->state[3] = 0xDA8A0600;
    ctx->state[4] = 0xA96F30BC;
    ctx->state[5] = 0x163138AA;
    ctx->state[6] = 0xE38DEE4D;
    ctx->state[7] = 0xB0FB0E4E;
    
    ctx->bitlen = 0;
    ctx->buflen = 0;
    memset(ctx->buffer, 0, SM3_BLOCK_SIZE);
}

/**
 * 更新SM3哈希计算
 * 处理任意长度的输入数据
 */
void sm3_update(sm3_ctx_t *ctx, const uint8_t *data, size_t len) {
    const uint8_t *ptr = data;
    size_t remaining = len;
    
    while (remaining > 0) {
        // 计算当前缓冲区还能装下多少字节
        size_t available = SM3_BLOCK_SIZE - ctx->buflen;
        size_t to_copy = (remaining < available) ? remaining : available;
        
        // 复制数据到缓冲区
        memcpy(ctx->buffer + ctx->buflen, ptr, to_copy);
        ctx->buflen += to_copy;
        ptr += to_copy;
        remaining -= to_copy;
        
        // 如果缓冲区满了，处理一个完整的块
        if (ctx->buflen == SM3_BLOCK_SIZE) {
            sm3_compress_basic(ctx->state, ctx->buffer);
            ctx->bitlen += SM3_BLOCK_SIZE * 8;
            ctx->buflen = 0;
        }
    }
}

/**
 * 完成SM3哈希计算
 * 添加填充并输出最终哈希值
 */
void sm3_final(sm3_ctx_t *ctx, uint8_t *digest) {
    uint64_t total_bits = ctx->bitlen + ctx->buflen * 8;
    uint8_t *buffer = ctx->buffer;
    uint32_t buflen = ctx->buflen;
    
    // 添加填充位
    buffer[buflen++] = 0x80;
    
    // 如果剩余空间不足8字节（存储长度），则填充到下一块
    if (buflen > 56) {
        while (buflen < 64) {
            buffer[buflen++] = 0x00;
        }
        sm3_compress_basic(ctx->state, buffer);
        buflen = 0;
    }
    
    // 填充0直到56字节位置
    while (buflen < 56) {
        buffer[buflen++] = 0x00;
    }
    
    // 添加原始消息长度（64位大端序）
    put_uint64_be(buffer + 56, total_bits);
    
    // 处理最后一个块
    sm3_compress_basic(ctx->state, buffer);
    
    // 输出哈希值（32字节，大端序）
    for (int i = 0; i < SM3_STATE_WORDS; i++) {
        put_uint32_be(digest + i * 4, ctx->state[i]);
    }
}

/**
 * One-shot SM3 hash computation
 * Convenience function for short messages
 */
void sm3_hash(const uint8_t *data, size_t len, uint8_t *digest) {
    sm3_ctx_t ctx;
    sm3_init(&ctx);
    sm3_update(&ctx, data, len);
    sm3_final(&ctx, digest);
}

/**
 * Basic version of SM3 hash computation
 * Uses standard implementation without special optimizations
 */
void sm3_basic_hash(const uint8_t *data, size_t len, uint8_t *digest) {
    sm3_ctx_t ctx;
    
    /* Initialize context */
    sm3_init(&ctx);
    
    /* Process input data */
    sm3_update(&ctx, data, len);
    
    /* Generate final hash value */
    sm3_final(&ctx, digest);
}

/**
 * Basic version of batch hash computation
 * Used for performance testing
 */
void sm3_basic_batch_hash(const uint8_t *data, size_t len, uint8_t *digest, int iterations) {
    for (int i = 0; i < iterations; i++) {
        sm3_basic_hash(data, len, digest);
        sm3_basic_hash(data, len, digest);
    }
}

/**
 * Print hash value (for debugging)
 */
void print_hash(const uint8_t *hash, const char *label) {
    printf("%s: ", label);
    for (int i = 0; i < SM3_DIGEST_SIZE; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
}

/**
 * Simple test for SM3 basic implementation
 */
int sm3_basic_test() {
    /* Test vector 1: empty string */
    const char *test1 = "";
    uint8_t expected1[] = {
        0x1a, 0xb2, 0x1d, 0x83, 0x55, 0xcf, 0xa1, 0x7f,
        0x8e, 0x61, 0x19, 0x48, 0x31, 0xe8, 0x1a, 0x8f,
        0x22, 0xbe, 0xc8, 0xc7, 0x28, 0xfe, 0xfb, 0x74,
        0x7e, 0xd0, 0x35, 0xeb, 0x50, 0x82, 0xaa, 0x2b
    };
    
    /* Test vector 2: "abc" */
    const char *test2 = "abc";
    uint8_t expected2[] = {
        0x66, 0xc7, 0xf0, 0xf4, 0x62, 0xee, 0xed, 0xd9,
        0xd1, 0xf2, 0xd4, 0x6b, 0xdc, 0x10, 0xe4, 0xe2,
        0x41, 0x67, 0xc4, 0x87, 0x5c, 0xf2, 0xf7, 0xa2,
        0x29, 0x7d, 0xa0, 0x2b, 0x8f, 0x4b, 0xa8, 0xe0
    };
    
    uint8_t digest[SM3_DIGEST_SIZE];
    int success = 1;
    
    printf("SM3 Basic Implementation Test...\n");
    
    /* Test 1 */
    sm3_basic_hash((const uint8_t *)test1, strlen(test1), digest);
    print_hash(digest, "Empty string");
    print_hash(expected1, "Expected");
    if (memcmp(digest, expected1, SM3_DIGEST_SIZE) != 0) {
        printf("❌ Test 1 failed\n");
        success = 0;
    } else {
        printf("✅ Test 1 passed\n");
    }
    printf("\n");
    
    /* Test 2 */
    sm3_basic_hash((const uint8_t *)test2, strlen(test2), digest);
    print_hash(digest, "String abc");
    print_hash(expected2, "Expected");
    if (memcmp(digest, expected2, SM3_DIGEST_SIZE) != 0) {
        printf("❌ Test 2 failed\n");
        success = 0;
    } else {
        printf("✅ Test 2 passed\n");
    }
    printf("\n");
    
    return success;
}

#ifdef STANDALONE_BASIC
/**
 * Standalone main function
 */
int main() {
    printf("=== SM3 Basic Implementation Test Program ===\n\n");
    
    int result = sm3_basic_test();
    
    if (result) {
        printf("🎉 All tests passed!\n");
        return 0;
    } else {
        printf("💥 Tests failed!\n");
        return 1;
    }
}
#endif
