#include "../common/sm3_common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * SM3 Length Extension Attack 实现
 * 
 * 原理：
 * SM3基于Merkle-Damgård构造，存在长度扩展攻击漏洞
 * 攻击者在不知道原始消息M的情况下，可以构造出
 * H(M || padding || M')的哈希值，其中M'是攻击者选择的消息
 * 
 * 攻击条件：
 * 1. 知道H(M)
 * 2. 知道M的长度
 * 3. 可以选择扩展消息M'
 */

// 计算SM3填充所需的长度
static size_t calculate_padding_length(size_t message_len) {
    size_t bit_len = message_len * 8;
    size_t padding_bits = 1; // 初始的1位
    
    // 计算需要多少个0位使得总长度 ≡ 448 (mod 512)
    size_t total_bits = bit_len + padding_bits;
    size_t zero_bits = (448 - (total_bits % 512) + 512) % 512;
    
    return (padding_bits + zero_bits + 64) / 8; // 转换为字节数（包括8字节长度字段）
}

// 构造SM3填充
static void construct_sm3_padding(uint8_t *padding, size_t original_len, size_t padding_len) {
    memset(padding, 0, padding_len);
    
    // 添加1位（0x80）
    padding[0] = 0x80;
    
    // 添加原始消息长度（64位大端序）
    uint64_t bit_len = original_len * 8;
    for (int i = 0; i < 8; i++) {
        padding[padding_len - 8 + i] = (uint8_t)(bit_len >> (56 - i * 8));
    }
}

/**
 * SM3长度扩展攻击实现
 * 
 * 输入：
 * - original_hash: 原始消息M的SM3哈希值（32字节）
 * - original_len: 原始消息M的长度
 * - extension: 要扩展的消息M'
 * - extension_len: 扩展消息M'的长度
 * 
 * 输出：
 * - forged_hash: 伪造的H(M || padding || M')哈希值
 * - forged_message: 完整的伪造消息（padding || M'）
 * - forged_message_len: 伪造消息的长度
 */
void sm3_length_extension_attack(
    const uint8_t original_hash[SM3_DIGEST_SIZE],
    size_t original_len,
    const uint8_t *extension,
    size_t extension_len,
    uint8_t forged_hash[SM3_DIGEST_SIZE],
    uint8_t **forged_message,
    size_t *forged_message_len
) {
    // 计算原始消息的填充
    size_t padding_len = calculate_padding_length(original_len);
    
    // 分配伪造消息内存
    *forged_message_len = padding_len + extension_len;
    *forged_message = malloc(*forged_message_len);
    if (!*forged_message) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    // 构造填充
    construct_sm3_padding(*forged_message, original_len, padding_len);
    
    // 添加扩展消息
    memcpy(*forged_message + padding_len, extension, extension_len);
    
    // 从原始哈希值恢复内部状态
    sm3_ctx_t ctx;
    
    // 将原始哈希值作为内部状态
    for (int i = 0; i < SM3_STATE_WORDS; i++) {
        ctx.state[i] = ((uint32_t)original_hash[i*4] << 24) |
                      ((uint32_t)original_hash[i*4+1] << 16) |
                      ((uint32_t)original_hash[i*4+2] << 8) |
                      ((uint32_t)original_hash[i*4+3]);
    }
    
    // 设置比特长度为原始消息+填充的长度
    ctx.bitlen = (original_len + padding_len) * 8;
    ctx.buflen = 0;
    
    // 处理扩展消息
    sm3_update(&ctx, extension, extension_len);
    
    // 生成最终哈希值
    sm3_final(&ctx, forged_hash);
}

/**
 * 验证长度扩展攻击
 * 
 * 验证伪造的哈希值是否等于直接计算H(original || forged_message)
 */
int verify_length_extension_attack(
    const uint8_t *original_message,
    size_t original_len,
    const uint8_t *forged_message,
    size_t forged_message_len,
    const uint8_t forged_hash[SM3_DIGEST_SIZE]
) {
    // 构造完整消息
    size_t total_len = original_len + forged_message_len;
    uint8_t *complete_message = malloc(total_len);
    if (!complete_message) {
        return 0;
    }
    
    memcpy(complete_message, original_message, original_len);
    memcpy(complete_message + original_len, forged_message, forged_message_len);
    
    // 计算完整消息的哈希值
    uint8_t computed_hash[SM3_DIGEST_SIZE];
    sm3_hash(complete_message, total_len, computed_hash);
    
    // 比较哈希值
    int result = (memcmp(forged_hash, computed_hash, SM3_DIGEST_SIZE) == 0);
    
    free(complete_message);
    return result;
}

/**
 * 打印长度扩展攻击演示
 */
void demonstrate_length_extension_attack(void) {
    printf("=== SM3 Length Extension Attack Demonstration ===\n\n");
    
    // 原始秘密消息（攻击者不知道内容，但知道长度）
    const char *secret = "secret_key";
    const char *known_suffix = "_public_data";
    
    // 构造原始消息
    size_t secret_len = strlen(secret);
    size_t suffix_len = strlen(known_suffix);
    size_t original_len = secret_len + suffix_len;
    
    uint8_t *original_message = malloc(original_len);
    memcpy(original_message, secret, secret_len);
    memcpy(original_message + secret_len, known_suffix, suffix_len);
    
    printf("Original message length: %zu bytes\n", original_len);
    printf("Original message (hex): ");
    for (size_t i = 0; i < original_len; i++) {
        printf("%02x", original_message[i]);
    }
    printf("\n\n");
    
    // 计算原始消息的哈希值
    uint8_t original_hash[SM3_DIGEST_SIZE];
    sm3_hash(original_message, original_len, original_hash);
    
    printf("Original hash: ");
    for (int i = 0; i < SM3_DIGEST_SIZE; i++) {
        printf("%02x", original_hash[i]);
    }
    printf("\n\n");
    
    // 攻击者要添加的恶意数据
    const char *malicious_data = "_admin_access";
    size_t malicious_len = strlen(malicious_data);
    
    printf("Malicious extension: %s\n", malicious_data);
    printf("Extension length: %zu bytes\n\n", malicious_len);
    
    // 执行长度扩展攻击
    uint8_t forged_hash[SM3_DIGEST_SIZE];
    uint8_t *forged_message;
    size_t forged_message_len;
    
    sm3_length_extension_attack(
        original_hash,
        original_len,
        (const uint8_t*)malicious_data,
        malicious_len,
        forged_hash,
        &forged_message,
        &forged_message_len
    );
    
    printf("Forged message (padding + extension, hex): ");
    for (size_t i = 0; i < forged_message_len; i++) {
        printf("%02x", forged_message[i]);
    }
    printf("\n\n");
    
    printf("Forged hash: ");
    for (int i = 0; i < SM3_DIGEST_SIZE; i++) {
        printf("%02x", forged_hash[i]);
    }
    printf("\n\n");
    
    // 验证攻击是否成功
    int attack_success = verify_length_extension_attack(
        original_message,
        original_len,
        forged_message,
        forged_message_len,
        forged_hash
    );
    
    printf("Attack verification: %s\n", attack_success ? "SUCCESS" : "FAILED");
    
    if (attack_success) {
        printf("\nAttack successful! The forged hash matches H(original || forged_message)\n");
        printf("This demonstrates that SM3 is vulnerable to length extension attacks.\n");
        printf("\nMitigation strategies:\n");
        printf("1. Use HMAC instead of bare hash functions for authentication\n");
        printf("2. Include the message length in the hash input\n");
        printf("3. Use hash functions with built-in length encoding (e.g., BLAKE2)\n");
    } else {
        printf("\nAttack failed - this should not happen with a correct implementation.\n");
    }
    
    // 清理内存
    free(original_message);
    free(forged_message);
    
    printf("\n=== End of Demonstration ===\n");
}
