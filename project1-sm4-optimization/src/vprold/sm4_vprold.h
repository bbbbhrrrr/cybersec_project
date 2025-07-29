/*
 * SM4算法VPROLD指令优化头文件
 * 利用Intel AVX-512的向量旋转指令
 */

#ifndef SM4_VPROLD_H
#define SM4_VPROLD_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 使用VPROLD指令优化的SM4单块加密
 * @param plaintext: 16字节明文输入
 * @param ciphertext: 16字节密文输出
 * @param round_keys: 32个轮密钥
 * @return: 成功返回0，失败返回负数
 */
int sm4_encrypt_block_vprold(const uint8_t *plaintext, uint8_t *ciphertext, 
                            const uint32_t *round_keys);

/*
 * 使用VPROLD指令优化的SM4单块解密
 * @param ciphertext: 16字节密文输入
 * @param plaintext: 16字节明文输出
 * @param round_keys: 32个轮密钥
 * @return: 成功返回0，失败返回负数
 */
int sm4_decrypt_block_vprold(const uint8_t *ciphertext, uint8_t *plaintext, 
                            const uint32_t *round_keys);

/*
 * 使用VPROLD指令的16块并行加密
 * @param plaintext: 256字节明文输入（16个块）
 * @param ciphertext: 256字节密文输出
 * @param round_keys: 32个轮密钥
 * @return: 成功返回0，失败返回负数
 */
int sm4_encrypt_16blocks_vprold(const uint8_t *plaintext, uint8_t *ciphertext, 
                               const uint32_t *round_keys);

/*
 * 使用VPROLD指令的ECB模式加密
 * @param plaintext: 明文输入
 * @param ciphertext: 密文输出
 * @param length: 数据长度（必须是16的倍数）
 * @param round_keys: 32个轮密钥
 * @return: 成功返回0，失败返回负数
 */
int sm4_ecb_encrypt_vprold(const uint8_t *plaintext, uint8_t *ciphertext, 
                          size_t length, const uint32_t *round_keys);

/*
 * 检查系统是否支持VPROLD指令
 * @return: 支持返回1，不支持返回0
 */
int sm4_vprold_available(void);

/*
 * 获取VPROLD优化信息
 * @return: 描述字符串
 */
const char* sm4_vprold_info(void);

#ifdef __cplusplus
}
#endif

#endif /* SM4_VPROLD_H */
