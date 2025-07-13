#include "sm3_common.h"
#include <string.h>

/* SM3算法初始值 */
const uint32_t SM3_IV[8] = {
    0x7380166F, 0x4914B2B9, 0x172442D7, 0xDA8A0600,
    0xA96F30BC, 0x163138AA, 0xE38DEE4D, 0xB0FB0E4E
};

/**
 * 初始化SM3上下文
 */
void sm3_init(sm3_ctx_t *ctx) {
    if (!ctx) return;
    
    /* 设置初始状态值 */
    memcpy(ctx->state, SM3_IV, sizeof(SM3_IV));
    
    /* 重置计数器和缓冲区 */
    ctx->bitlen = 0;
    ctx->buflen = 0;
    memset(ctx->buffer, 0, sizeof(ctx->buffer));
}

/**
 * 字节序转换函数
 */
static void bytes_to_words(const uint8_t *bytes, uint32_t *words, size_t word_count) {
    for (size_t i = 0; i < word_count; i++) {
        words[i] = ((uint32_t)bytes[i * 4] << 24) |
                   ((uint32_t)bytes[i * 4 + 1] << 16) |
                   ((uint32_t)bytes[i * 4 + 2] << 8) |
                   ((uint32_t)bytes[i * 4 + 3]);
    }
}

/**
 * 字节序转换函数（反向）
 */
static void words_to_bytes(const uint32_t *words, uint8_t *bytes, size_t word_count) {
    for (size_t i = 0; i < word_count; i++) {
        bytes[i * 4] = (uint8_t)(words[i] >> 24);
        bytes[i * 4 + 1] = (uint8_t)(words[i] >> 16);
        bytes[i * 4 + 2] = (uint8_t)(words[i] >> 8);
        bytes[i * 4 + 3] = (uint8_t)(words[i]);
    }
}

/**
 * SM3消息扩展函数
 */
static void sm3_expand(const uint32_t X[16], uint32_t W[68], uint32_t W1[64]) {
    int j;
    
    /* 前16个字直接来自输入 */
    for (j = 0; j < 16; j++) {
        W[j] = X[j];
    }
    
    /* 扩展后52个字 */
    for (j = 16; j < 68; j++) {
        W[j] = P1(W[j-16] ^ W[j-9] ^ ROL32(W[j-3], 15)) ^ ROL32(W[j-13], 7) ^ W[j-6];
    }
    
    /* 计算W1 */
    for (j = 0; j < 64; j++) {
        W1[j] = W[j] ^ W[j+4];
    }
}

/**
 * SM3压缩函数（基础实现）
 */
void sm3_compress(uint32_t state[8], const uint8_t block[64]) {
    uint32_t W[68], W1[64];
    uint32_t X[16];
    uint32_t A, B, C, D, E, F, G, H;
    uint32_t SS1, SS2, TT1, TT2, T;
    int j;
    
    /* 将64字节分组转换为16个32位字 */
    bytes_to_words(block, X, 16);
    
    /* 消息扩展 */
    sm3_expand(X, W, W1);
    
    /* 初始化工作变量 */
    A = state[0];
    B = state[1];
    C = state[2];
    D = state[3];
    E = state[4];
    F = state[5];
    G = state[6];
    H = state[7];
    
    /* 64轮压缩函数 */
    for (j = 0; j < 64; j++) {
        /* 计算T值 */
        T = (j < 16) ? SM3_T1 : SM3_T2;
        
        /* 计算SS1和SS2 */
        SS1 = ROL32(ROL32(A, 12) + E + ROL32(T, j % 32), 7);
        SS2 = SS1 ^ ROL32(A, 12);
        
        /* 计算TT1和TT2 */
        if (j < 16) {
            TT1 = FF0(A, B, C) + D + SS2 + W1[j];
            TT2 = GG0(E, F, G) + H + SS1 + W[j];
        } else {
            TT1 = FF1(A, B, C) + D + SS2 + W1[j];
            TT2 = GG1(E, F, G) + H + SS1 + W[j];
        }
        
        /* 更新工作变量 */
        D = C;
        C = ROL32(B, 9);
        B = A;
        A = TT1;
        H = G;
        G = ROL32(F, 19);
        F = E;
        E = P0(TT2);
        
        /* 调试输出（可选） */
        /* printf("j=%d: A=%08X B=%08X C=%08X D=%08X E=%08X F=%08X G=%08X H=%08X\n",
               j, A, B, C, D, E, F, G, H); */
    }
    
    /* 更新状态 */
    state[0] ^= A;
    state[1] ^= B;
    state[2] ^= C;
    state[3] ^= D;
    state[4] ^= E;
    state[5] ^= F;
    state[6] ^= G;
    state[7] ^= H;
}

/**
 * 处理填充和最后一个分组
 */
static void sm3_pad_and_process(sm3_ctx_t *ctx) {
    uint64_t bitlen = ctx->bitlen;
    uint32_t msglen = ctx->buflen;
    uint32_t padlen;
    
    /* 添加填充字节 0x80 */
    ctx->buffer[msglen++] = 0x80;
    
    /* 计算需要的填充长度 */
    if (msglen > SM3_BLOCK_SIZE - 8) {
        /* 需要额外的一个分组 */
        padlen = SM3_BLOCK_SIZE - msglen;
        memset(ctx->buffer + msglen, 0, padlen);
        sm3_compress(ctx->state, ctx->buffer);
        
        /* 准备最后一个分组（仅包含长度） */
        memset(ctx->buffer, 0, SM3_BLOCK_SIZE - 8);
        msglen = SM3_BLOCK_SIZE - 8;
    } else {
        /* 在当前分组中填充到56字节 */
        padlen = SM3_BLOCK_SIZE - 8 - msglen;
        memset(ctx->buffer + msglen, 0, padlen);
        msglen = SM3_BLOCK_SIZE - 8;
    }
    
    /* 添加64位长度字段（大端序） */
    ctx->buffer[msglen++] = (uint8_t)(bitlen >> 56);
    ctx->buffer[msglen++] = (uint8_t)(bitlen >> 48);
    ctx->buffer[msglen++] = (uint8_t)(bitlen >> 40);
    ctx->buffer[msglen++] = (uint8_t)(bitlen >> 32);
    ctx->buffer[msglen++] = (uint8_t)(bitlen >> 24);
    ctx->buffer[msglen++] = (uint8_t)(bitlen >> 16);
    ctx->buffer[msglen++] = (uint8_t)(bitlen >> 8);
    ctx->buffer[msglen++] = (uint8_t)(bitlen);
    
    /* 处理最后一个分组 */
    sm3_compress(ctx->state, ctx->buffer);
}

/**
 * 更新SM3哈希计算
 */
void sm3_update(sm3_ctx_t *ctx, const uint8_t *data, size_t len) {
    if (!ctx || !data) return;
    
    const uint8_t *ptr = data;
    size_t remaining = len;
    
    /* 更新总位数 */
    ctx->bitlen += len * 8;
    
    /* 处理缓冲区中的剩余数据 */
    if (ctx->buflen > 0) {
        size_t to_copy = SM3_BLOCK_SIZE - ctx->buflen;
        if (to_copy > remaining) {
            to_copy = remaining;
        }
        
        memcpy(ctx->buffer + ctx->buflen, ptr, to_copy);
        ctx->buflen += to_copy;
        ptr += to_copy;
        remaining -= to_copy;
        
        /* 如果缓冲区填满，处理这个分组 */
        if (ctx->buflen == SM3_BLOCK_SIZE) {
            sm3_compress(ctx->state, ctx->buffer);
            ctx->buflen = 0;
        }
    }
    
    /* 处理完整的64字节分组 */
    while (remaining >= SM3_BLOCK_SIZE) {
        sm3_compress(ctx->state, ptr);
        ptr += SM3_BLOCK_SIZE;
        remaining -= SM3_BLOCK_SIZE;
    }
    
    /* 保存剩余数据到缓冲区 */
    if (remaining > 0) {
        memcpy(ctx->buffer, ptr, remaining);
        ctx->buflen = remaining;
    }
}

/**
 * 完成SM3哈希计算
 */
void sm3_final(sm3_ctx_t *ctx, uint8_t *digest) {
    if (!ctx || !digest) return;
    
    /* 处理填充和最后的分组 */
    sm3_pad_and_process(ctx);
    
    /* 输出最终哈希值 */
    words_to_bytes(ctx->state, digest, SM3_STATE_WORDS);
    
    /* 清理上下文（安全考虑） */
    memset(ctx, 0, sizeof(sm3_ctx_t));
}

/**
 * 一次性计算SM3哈希值
 */
void sm3_hash(const uint8_t *data, size_t len, uint8_t *digest) {
    sm3_ctx_t ctx;
    
    sm3_init(&ctx);
    sm3_update(&ctx, data, len);
    sm3_final(&ctx, digest);
}
