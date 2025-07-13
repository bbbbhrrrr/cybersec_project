#ifndef SM3_COMMON_H
#define SM3_COMMON_H

#include <stdint.h>
#include <stddef.h>

/* SM3算法常量定义 */
#define SM3_DIGEST_SIZE     32      /* SM3输出长度（字节） */
#define SM3_BLOCK_SIZE      64      /* SM3分组长度（字节） */
#define SM3_WORD_SIZE       4       /* 字长度（字节） */
#define SM3_STATE_WORDS     8       /* 状态字数量 */
#define SM3_BLOCK_WORDS     16      /* 分组字数量 */

/* SM3状态结构体 */
typedef struct {
    uint32_t state[SM3_STATE_WORDS];    /* 8个32位状态字 */
    uint8_t buffer[SM3_BLOCK_SIZE];     /* 输入缓冲区 */
    uint64_t bitlen;                    /* 已处理的位数 */
    uint32_t buflen;                    /* 缓冲区数据长度 */
} sm3_ctx_t;

/* SM3算法接口函数声明 */

/**
 * 初始化SM3上下文
 * @param ctx SM3上下文指针
 */
void sm3_init(sm3_ctx_t *ctx);

/**
 * 更新SM3哈希计算（处理输入数据）
 * @param ctx SM3上下文指针
 * @param data 输入数据指针
 * @param len 输入数据长度
 */
void sm3_update(sm3_ctx_t *ctx, const uint8_t *data, size_t len);

/**
 * 完成SM3哈希计算（输出最终哈希值）
 * @param ctx SM3上下文指针
 * @param digest 输出哈希值缓冲区（至少32字节）
 */
void sm3_final(sm3_ctx_t *ctx, uint8_t *digest);

/**
 * 一次性计算SM3哈希值
 * @param data 输入数据指针
 * @param len 输入数据长度
 * @param digest 输出哈希值缓冲区（至少32字节）
 */
void sm3_hash(const uint8_t *data, size_t len, uint8_t *digest);

/* SM3算法内部函数声明 */

/**
 * SM3压缩函数
 * @param state 当前状态
 * @param block 512位输入分组
 */
void sm3_compress(uint32_t state[8], const uint8_t block[64]);

/* SM3算法相关常量 */

/* T常数 */
#define SM3_T1  0x79CC4519U     /* T_j for j = 0, 1, ..., 15 */
#define SM3_T2  0x7A879D8AU     /* T_j for j = 16, 17, ..., 63 */

/* 初始值IV */
extern const uint32_t SM3_IV[8];

/* 布尔函数宏定义 */
#define FF0(x, y, z) ((x) ^ (y) ^ (z))
#define FF1(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define GG0(x, y, z) ((x) ^ (y) ^ (z))
#define GG1(x, y, z) (((x) & (y)) | ((~(x)) & (z)))

/* 置换函数宏定义 */
#define P0(x) ((x) ^ ROL32((x), 9) ^ ROL32((x), 17))
#define P1(x) ((x) ^ ROL32((x), 15) ^ ROL32((x), 23))

/* 循环左移宏定义 */
#define ROL32(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

/* 字节序转换宏定义 */
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define SWAP32(x) __builtin_bswap32(x)
#else
#define SWAP32(x) (x)
#endif

/* 错误代码定义 */
#define SM3_SUCCESS         0
#define SM3_ERROR_NULL_PTR  -1
#define SM3_ERROR_INVALID   -2

#endif /* SM3_COMMON_H */
