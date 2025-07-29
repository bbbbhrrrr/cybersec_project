/*
 * SM4-GCM (Galois/Counter Mode) 认证加密模式头文件
 * 提供SM4算法的GCM工作模式接口
 */

#ifndef SM4_GCM_H
#define SM4_GCM_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// GCM模式常量
#define SM4_GCM_BLOCK_SIZE  16
#define SM4_GCM_KEY_SIZE    16
#define SM4_GCM_IV_SIZE     12
#define SM4_GCM_TAG_SIZE    16

// GCM上下文结构
typedef struct {
    uint32_t round_keys[32];        // SM4轮密钥
    uint8_t h[16];                  // GHASH密钥H = E_K(0^128)
    uint8_t j0[16];                 // 初始计数器
    uint8_t counter[16];            // 当前计数器
    uint8_t ghash_state[16];        // GHASH状态
    size_t aad_len;                 // 附加认证数据长度
    size_t counter_len;             // 计数器长度
    int initialized;                // 初始化标志
} sm4_gcm_context;

// 错误代码
#define SM4_GCM_SUCCESS         0
#define SM4_GCM_ERROR_PARAM     -1
#define SM4_GCM_ERROR_INIT      -2
#define SM4_GCM_ERROR_START     -3
#define SM4_GCM_ERROR_UPDATE    -4
#define SM4_GCM_ERROR_FINISH    -5
#define SM4_GCM_ERROR_VERIFY    -6

/*
 * 初始化GCM上下文
 * @param ctx: GCM上下文
 * @param key: 128位密钥
 * @return: 成功返回0，失败返回错误代码
 */
int sm4_gcm_init(sm4_gcm_context *ctx, const uint8_t *key);

/*
 * 开始GCM加密/解密操作
 * @param ctx: GCM上下文
 * @param iv: 初始化向量
 * @param iv_len: IV长度（建议12字节）
 * @param aad: 附加认证数据
 * @param aad_len: AAD长度
 * @return: 成功返回0，失败返回错误代码
 */
int sm4_gcm_start(sm4_gcm_context *ctx, const uint8_t *iv, size_t iv_len, 
                  const uint8_t *aad, size_t aad_len);

/*
 * GCM加密更新
 * @param ctx: GCM上下文
 * @param plaintext: 明文输入
 * @param ciphertext: 密文输出
 * @param length: 数据长度
 * @return: 成功返回0，失败返回错误代码
 */
int sm4_gcm_encrypt_update(sm4_gcm_context *ctx, const uint8_t *plaintext, 
                          uint8_t *ciphertext, size_t length);

/*
 * GCM解密更新
 * @param ctx: GCM上下文
 * @param ciphertext: 密文输入
 * @param plaintext: 明文输出
 * @param length: 数据长度
 * @return: 成功返回0，失败返回错误代码
 */
int sm4_gcm_decrypt_update(sm4_gcm_context *ctx, const uint8_t *ciphertext, 
                          uint8_t *plaintext, size_t length);

/*
 * 完成GCM操作并生成认证标签
 * @param ctx: GCM上下文
 * @param tag: 输出认证标签（16字节）
 * @return: 成功返回0，失败返回错误代码
 */
int sm4_gcm_finish(sm4_gcm_context *ctx, uint8_t *tag);

/*
 * 验证认证标签
 * @param tag1: 标签1
 * @param tag2: 标签2
 * @return: 标签匹配返回0，不匹配返回-1
 */
int sm4_gcm_verify_tag(const uint8_t *tag1, const uint8_t *tag2);

/*
 * 一次性GCM加密函数
 * @param key: 128位密钥
 * @param iv: 初始化向量
 * @param iv_len: IV长度
 * @param aad: 附加认证数据
 * @param aad_len: AAD长度
 * @param plaintext: 明文输入
 * @param ciphertext: 密文输出
 * @param length: 明文长度
 * @param tag: 输出认证标签
 * @return: 成功返回0，失败返回错误代码
 */
int sm4_gcm_encrypt(const uint8_t *key, const uint8_t *iv, size_t iv_len,
                   const uint8_t *aad, size_t aad_len,
                   const uint8_t *plaintext, uint8_t *ciphertext, size_t length,
                   uint8_t *tag);

/*
 * 一次性GCM解密函数
 * @param key: 128位密钥
 * @param iv: 初始化向量
 * @param iv_len: IV长度
 * @param aad: 附加认证数据
 * @param aad_len: AAD长度
 * @param ciphertext: 密文输入
 * @param plaintext: 明文输出
 * @param length: 密文长度
 * @param tag: 认证标签
 * @return: 成功返回0，失败返回错误代码
 */
int sm4_gcm_decrypt(const uint8_t *key, const uint8_t *iv, size_t iv_len,
                   const uint8_t *aad, size_t aad_len,
                   const uint8_t *ciphertext, uint8_t *plaintext, size_t length,
                   const uint8_t *tag);

#ifdef __cplusplus
}
#endif

#endif /* SM4_GCM_H */
