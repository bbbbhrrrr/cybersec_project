#ifndef SM4_SIMD_H
#define SM4_SIMD_H

#include "../common/sm4.h"

#ifdef __cplusplus
extern "C" {
#endif

// SIMD batch processing functions
int sm4_encrypt_blocks_simd(const sm4_context_t *ctx, const uint8_t *input, uint8_t *output, size_t num_blocks);
int sm4_decrypt_blocks_simd(const sm4_context_t *ctx, const uint8_t *input, uint8_t *output, size_t num_blocks);

#ifdef __cplusplus
}
#endif

#endif // SM4_SIMD_H
