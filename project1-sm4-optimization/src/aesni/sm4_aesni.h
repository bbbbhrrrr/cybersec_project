/*
 * SM4 AES-NI 优化实现头文件
 */

#ifndef SM4_AESNI_H
#define SM4_AESNI_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 使用 AES-NI 指令集优化的 SM4 单块加密
 * 
 * @param plaintext: 16字节明文输入
 * @param ciphertext: 16字节密文输出
 * @param round_keys: 32个轮密钥
 * @return: 0成功，-1不支持AES-NI
 */
int sm4_aesni_encrypt_block(const uint8_t *plaintext, uint8_t *ciphertext, const uint32_t *round_keys);

/*
 * 使用 AES-NI 指令集优化的 SM4 单块解密
 * 
 * @param ciphertext: 16字节密文输入
 * @param plaintext: 16字节明文输出
 * @param round_keys: 32个轮密钥
 * @return: 0成功，-1不支持AES-NI
 */
int sm4_aesni_decrypt_block(const uint8_t *ciphertext, uint8_t *plaintext, const uint32_t *round_keys);

/*
 * AES-NI 优化的4块并行加密
 * 
 * @param plaintext: 64字节明文输入（4个块）
 * @param ciphertext: 64字节密文输出（4个块）
 * @param round_keys: 32个轮密钥
 * @return: 0成功，-1不支持AES-NI
 */
int sm4_aesni_encrypt_4blocks(const uint8_t *plaintext, uint8_t *ciphertext, const uint32_t *round_keys);

/*
 * AES-NI 优化的 ECB 模式加密
 * 
 * @param plaintext: 明文输入
 * @param ciphertext: 密文输出
 * @param length: 数据长度（必须是16的倍数）
 * @param round_keys: 32个轮密钥
 * @return: 0成功，-1不支持AES-NI，-2长度错误
 */
int sm4_aesni_encrypt_ecb(const uint8_t *plaintext, uint8_t *ciphertext, size_t length, const uint32_t *round_keys);

/*
 * AES-NI 优化的 ECB 模式解密
 * 
 * @param ciphertext: 密文输入
 * @param plaintext: 明文输出
 * @param length: 数据长度（必须是16的倍数）
 * @param round_keys: 32个轮密钥
 * @return: 0成功，-1不支持AES-NI，-2长度错误
 */
int sm4_aesni_decrypt_ecb(const uint8_t *ciphertext, uint8_t *plaintext, size_t length, const uint32_t *round_keys);

#ifdef __cplusplus
}
#endif

#endif /* SM4_AESNI_H */
