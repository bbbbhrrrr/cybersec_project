#ifndef SM3_H
#define SM3_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// SM3 常量定义
#define SM3_DIGEST_SIZE 32
#define SM3_BLOCK_SIZE  64
#define SM3_STATE_SIZE  8

// SM3 上下文结构
typedef struct {
    uint32_t state[SM3_STATE_SIZE];  // 哈希状态 A,B,C,D,E,F,G,H
    uint8_t  buffer[SM3_BLOCK_SIZE]; // 输入缓冲区
    uint64_t count;                  // 处理的比特数
    size_t   buffer_len;             // 缓冲区中的字节数
} sm3_ctx_t;

// 基础SM3函数
void sm3_init(sm3_ctx_t *ctx);
void sm3_update(sm3_ctx_t *ctx, const uint8_t *data, size_t len);
void sm3_final(sm3_ctx_t *ctx, uint8_t digest[SM3_DIGEST_SIZE]);

// 便捷函数
void sm3_hash(const uint8_t *data, size_t len, uint8_t digest[SM3_DIGEST_SIZE]);

// 内部函数（用于优化版本）
void sm3_compress(uint32_t state[SM3_STATE_SIZE], const uint8_t block[SM3_BLOCK_SIZE]);

// 调试和测试函数
void sm3_print_state(const uint32_t state[SM3_STATE_SIZE]);
void sm3_print_hex(const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif // SM3_H
