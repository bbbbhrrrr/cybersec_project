/*
 * SM4 GFNI (Galois Field New Instructions) 优化实现头文件
 */

#ifndef SM4_GFNI_H
#define SM4_GFNI_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 使用 GFNI 指令集优化的 SM4 单块加密
 * 
 * @param plaintext: 16字节明文输入
 * @param ciphertext: 16字节密文输出
 * @param round_keys: 32个轮密钥
 * @return: 0成功，-1不支持GFNI
 */
int sm4_gfni_encrypt_block(const uint8_t *plaintext, uint8_t *ciphertext, const uint32_t *round_keys);

/*
 * 使用 GFNI 指令集优化的 SM4 单块解密
 * 
 * @param ciphertext: 16字节密文输入
 * @param plaintext: 16字节明文输出
 * @param round_keys: 32个轮密钥
 * @return: 0成功，-1不支持GFNI
 */
int sm4_gfni_decrypt_block(const uint8_t *ciphertext, uint8_t *plaintext, const uint32_t *round_keys);

/*
 * GFNI + AVX-512 优化的8块并行加密
 * 
 * @param plaintext: 128字节明文输入（8个块）
 * @param ciphertext: 128字节密文输出（8个块）
 * @param round_keys: 32个轮密钥
 * @return: 0成功，-1不支持GFNI/AVX-512
 */
int sm4_gfni_encrypt_8blocks_avx512(const uint8_t *plaintext, uint8_t *ciphertext, const uint32_t *round_keys);

/*
 * GFNI 优化的 ECB 模式加密
 * 
 * @param plaintext: 明文输入
 * @param ciphertext: 密文输出
 * @param length: 数据长度（必须是16的倍数）
 * @param round_keys: 32个轮密钥
 * @return: 0成功，-1不支持GFNI，-2长度错误
 */
int sm4_gfni_encrypt_ecb(const uint8_t *plaintext, uint8_t *ciphertext, size_t length, const uint32_t *round_keys);

/*
 * GFNI 优化的 ECB 模式解密
 * 
 * @param ciphertext: 密文输入
 * @param plaintext: 明文输出
 * @param length: 数据长度（必须是16的倍数）
 * @param round_keys: 32个轮密钥
 * @return: 0成功，-1不支持GFNI，-2长度错误
 */
int sm4_gfni_decrypt_ecb(const uint8_t *ciphertext, uint8_t *plaintext, size_t length, const uint32_t *round_keys);

#ifdef __cplusplus
}
#endif

#endif /* SM4_GFNI_H */
