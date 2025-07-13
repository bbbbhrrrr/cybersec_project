#include "../common/sm3_common.h"
#include <stdio.h>
#include <string.h>

/**
 * SM3基础实现 - 标准版本
 * 严格按照GM/T 0004-2012标准实现
 */

/**
 * 基础版本的SM3哈希计算
 * 使用标准实现，无特殊优化
 */
void sm3_basic_hash(const uint8_t *data, size_t len, uint8_t *digest) {
    sm3_ctx_t ctx;
    
    /* 初始化上下文 */
    sm3_init(&ctx);
    
    /* 处理输入数据 */
    sm3_update(&ctx, data, len);
    
    /* 生成最终哈希值 */
    sm3_final(&ctx, digest);
}

/**
 * 基础版本的批量哈希计算
 * 用于性能测试
 */
void sm3_basic_batch_hash(const uint8_t *data, size_t len, uint8_t *digest, int iterations) {
    for (int i = 0; i < iterations; i++) {
        sm3_basic_hash(data, len, digest);
    }
}

/**
 * 打印哈希值（调试用）
 */
void print_hash(const uint8_t *hash, const char *label) {
    printf("%s: ", label);
    for (int i = 0; i < SM3_DIGEST_SIZE; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
}

/**
 * SM3基础实现的简单测试
 */
int sm3_basic_test() {
    /* 测试向量1: 空字符串 */
    const char *test1 = "";
    uint8_t expected1[] = {
        0x1a, 0xb2, 0x1d, 0x83, 0x55, 0xcf, 0xa1, 0x7f,
        0x8e, 0x61, 0x19, 0x48, 0x31, 0xe8, 0x1a, 0x8f,
        0x22, 0xbe, 0xc8, 0xc7, 0x28, 0xfe, 0xfb, 0x74,
        0x7e, 0xd0, 0x35, 0xeb, 0x50, 0x82, 0xaa, 0x2b
    };
    
    /* 测试向量2: "abc" */
    const char *test2 = "abc";
    uint8_t expected2[] = {
        0x66, 0xc7, 0xf0, 0xf4, 0x62, 0xee, 0xed, 0xd9,
        0xd1, 0xf2, 0xd4, 0x6b, 0xdc, 0x10, 0xe4, 0xe2,
        0x41, 0x67, 0xc4, 0x87, 0x5c, 0xf2, 0xf7, 0xa2,
        0x29, 0x7d, 0xa0, 0x2b, 0x8f, 0x4b, 0xa8, 0xe0
    };
    
    uint8_t digest[SM3_DIGEST_SIZE];
    int success = 1;
    
    printf("SM3基础实现测试...\n");
    
    /* 测试1 */
    sm3_basic_hash((const uint8_t *)test1, strlen(test1), digest);
    print_hash(digest, "空字符串");
    print_hash(expected1, "期望值");
    if (memcmp(digest, expected1, SM3_DIGEST_SIZE) != 0) {
        printf("❌ 测试1失败\n");
        success = 0;
    } else {
        printf("✅ 测试1通过\n");
    }
    printf("\n");
    
    /* 测试2 */
    sm3_basic_hash((const uint8_t *)test2, strlen(test2), digest);
    print_hash(digest, "字符串abc");
    print_hash(expected2, "期望值");
    if (memcmp(digest, expected2, SM3_DIGEST_SIZE) != 0) {
        printf("❌ 测试2失败\n");
        success = 0;
    } else {
        printf("✅ 测试2通过\n");
    }
    printf("\n");
    
    return success;
}

#ifdef STANDALONE_BASIC
/**
 * 独立运行的主函数
 */
int main() {
    printf("=== SM3基础实现测试程序 ===\n\n");
    
    int result = sm3_basic_test();
    
    if (result) {
        printf("🎉 所有测试通过！\n");
        return 0;
    } else {
        printf("💥 测试失败！\n");
        return 1;
    }
}
#endif
