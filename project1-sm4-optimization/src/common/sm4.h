#ifndef SM4_H
#define SM4_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// SM4 Constants
#define SM4_BLOCK_SIZE 16
#define SM4_KEY_SIZE 16
#define SM4_ROUNDS 32

// SM4 Context Structure
typedef struct {
    uint32_t rk[SM4_ROUNDS];  // Round keys
} sm4_context_t;

// Function declarations
int sm4_set_encrypt_key(sm4_context_t *ctx, const uint8_t key[SM4_KEY_SIZE]);
int sm4_set_decrypt_key(sm4_context_t *ctx, const uint8_t key[SM4_KEY_SIZE]);
int sm4_encrypt_block(const sm4_context_t *ctx, const uint8_t input[SM4_BLOCK_SIZE], uint8_t output[SM4_BLOCK_SIZE]);
int sm4_decrypt_block(const sm4_context_t *ctx, const uint8_t input[SM4_BLOCK_SIZE], uint8_t output[SM4_BLOCK_SIZE]);

// Utility functions
void print_hex(const uint8_t *data, size_t len);
void print_block(const char *label, const uint8_t *block);

#ifdef __cplusplus
}
#endif

#endif // SM4_H
