/*
 * SM4算法使用VPROLD指令的优化实现
 * 利用Intel AVX-512的向量旋转指令提升性能
 */

#include "sm4_vprold.h"
#include "../common/sm4_common.h"
#include <immintrin.h>
#include <string.h>

// 检测VPROLD指令支持
static int vprold_supported = -1;

// 检测CPU是否支持AVX-512和VPROLD指令
static int check_vprold_support(void) {
    if (vprold_supported != -1) {
        return vprold_supported;
    }
    
    unsigned int eax, ebx, ecx, edx;
    
    // 检查CPUID支持
    __asm__ __volatile__("cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0));
    
    if (eax < 7) {
        vprold_supported = 0;
        return 0;
    }
    
    // 检查AVX-512F支持 (CPUID.07H:EBX[16])
    __asm__ __volatile__("cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(7), "c"(0));
    
    if (!(ebx & (1 << 16))) {
        vprold_supported = 0;
        return 0;
    }
    
    // 检查VPROLD指令支持 (通常包含在AVX-512F中)
    vprold_supported = 1;
    return 1;
}

// 使用VPROLD优化的S盒查找
static __m512i sm4_sbox_vprold(__m512i input) {
    // SM4 S盒
    static const uint8_t sbox[256] = {
        0xd6, 0x90, 0xe9, 0xfe, 0xcc, 0xe1, 0x3d, 0xb7, 0x16, 0xb6, 0x14, 0xc2, 0x28, 0xfb, 0x2c, 0x05,
        0x2b, 0x67, 0x9a, 0x76, 0x2a, 0xbe, 0x04, 0xc3, 0xaa, 0x44, 0x13, 0x26, 0x49, 0x86, 0x06, 0x99,
        0x9c, 0x42, 0x50, 0xf4, 0x91, 0xef, 0x98, 0x7a, 0x33, 0x54, 0x0b, 0x43, 0xed, 0xcf, 0xac, 0x62,
        0xe4, 0xb3, 0x1c, 0xa9, 0xc9, 0x08, 0xe8, 0x95, 0x80, 0xdf, 0x94, 0xfa, 0x75, 0x8f, 0x3f, 0xa6,
        0x47, 0x07, 0xa7, 0xfc, 0xf3, 0x73, 0x17, 0xba, 0x83, 0x59, 0x3c, 0x19, 0xe6, 0x85, 0x4f, 0xa8,
        0x68, 0x6b, 0x81, 0xb2, 0x71, 0x64, 0xda, 0x8b, 0xf8, 0xeb, 0x0f, 0x4b, 0x70, 0x56, 0x9d, 0x35,
        0x1e, 0x24, 0x0e, 0x5e, 0x63, 0x58, 0xd1, 0xa2, 0x25, 0x22, 0x7c, 0x3b, 0x01, 0x21, 0x78, 0x87,
        0xd4, 0x00, 0x46, 0x57, 0x9f, 0xd3, 0x27, 0x52, 0x4c, 0x36, 0x02, 0xe7, 0xa0, 0xc4, 0xc8, 0x9e,
        0xea, 0xbf, 0x8a, 0xd2, 0x40, 0xc7, 0x38, 0xb5, 0xa3, 0xf7, 0xf2, 0xce, 0xf9, 0x61, 0x15, 0xa1,
        0xe0, 0xae, 0x5d, 0xa4, 0x9b, 0x34, 0x1a, 0x55, 0xad, 0x93, 0x32, 0x30, 0xf5, 0x8c, 0xb1, 0xe3,
        0x1d, 0xf6, 0xe2, 0x2e, 0x82, 0x66, 0xca, 0x60, 0xc0, 0x29, 0x23, 0xab, 0x0d, 0x53, 0x4e, 0x6f,
        0xd5, 0xdb, 0x37, 0x45, 0xde, 0xfd, 0x8e, 0x2f, 0x03, 0xff, 0x6a, 0x72, 0x6d, 0x6c, 0x5b, 0x51,
        0x8d, 0x1b, 0xaf, 0x92, 0xbb, 0xdd, 0xbc, 0x7f, 0x11, 0xd9, 0x5c, 0x41, 0x1f, 0x10, 0x5a, 0xd8,
        0x0a, 0xc1, 0x31, 0x88, 0xa5, 0xcd, 0x7b, 0xbd, 0x2d, 0x74, 0xd0, 0x12, 0xb8, 0xe5, 0xb4, 0xb0,
        0x89, 0x69, 0x97, 0x4a, 0x0c, 0x96, 0x77, 0x7e, 0x65, 0xb9, 0xf1, 0x09, 0xc5, 0x6e, 0xc6, 0x84,
        0x18, 0xf0, 0x7d, 0xec, 0x3a, 0xdc, 0x4d, 0x20, 0x79, 0xee, 0x5f, 0x3e, 0xd7, 0xcb, 0x39, 0x48
    };
    
    // 将16个32位值分别进行S盒查找
    uint32_t values[16];
    _mm512_storeu_si512((__m512i*)values, input);
    
    for (int i = 0; i < 16; i++) {
        uint32_t x = values[i];
        values[i] = (sbox[(x >> 24) & 0xFF] << 24) |
                   (sbox[(x >> 16) & 0xFF] << 16) |
                   (sbox[(x >> 8) & 0xFF] << 8) |
                   (sbox[x & 0xFF]);
    }
    
    return _mm512_loadu_si512((__m512i*)values);
}

// 使用VPROLD的线性变换
static __m512i sm4_linear_transform_vprold(__m512i input) {
    // L(B) = B XOR (B <<< 2) XOR (B <<< 10) XOR (B <<< 18) XOR (B <<< 24)
    __m512i b = input;
    __m512i b2 = _mm512_rol_epi32(b, 2);   // VPROLD指令
    __m512i b10 = _mm512_rol_epi32(b, 10);
    __m512i b18 = _mm512_rol_epi32(b, 18);
    __m512i b24 = _mm512_rol_epi32(b, 24);
    
    __m512i result = _mm512_xor_si512(b, b2);
    result = _mm512_xor_si512(result, b10);
    result = _mm512_xor_si512(result, b18);
    result = _mm512_xor_si512(result, b24);
    
    return result;
}

// 使用VPROLD的密钥扩展线性变换
static __m512i sm4_key_linear_transform_vprold(__m512i input) {
    // L'(B) = B XOR (B <<< 13) XOR (B <<< 23)
    __m512i b = input;
    __m512i b13 = _mm512_rol_epi32(b, 13);  // VPROLD指令
    __m512i b23 = _mm512_rol_epi32(b, 23);
    
    __m512i result = _mm512_xor_si512(b, b13);
    result = _mm512_xor_si512(result, b23);
    
    return result;
}

// VPROLD优化的单块加密
int sm4_encrypt_block_vprold(const uint8_t *plaintext, uint8_t *ciphertext, 
                            const uint32_t *round_keys) {
    if (!check_vprold_support()) {
        return -1;  // 不支持VPROLD指令
    }
    
    if (!plaintext || !ciphertext || !round_keys) {
        return -2;
    }
    
    // 加载明文
    uint32_t x0 = (plaintext[0] << 24) | (plaintext[1] << 16) | 
                  (plaintext[2] << 8) | plaintext[3];
    uint32_t x1 = (plaintext[4] << 24) | (plaintext[5] << 16) | 
                  (plaintext[6] << 8) | plaintext[7];
    uint32_t x2 = (plaintext[8] << 24) | (plaintext[9] << 16) | 
                  (plaintext[10] << 8) | plaintext[11];
    uint32_t x3 = (plaintext[12] << 24) | (plaintext[13] << 16) | 
                  (plaintext[14] << 8) | plaintext[15];
    
    // 32轮迭代
    for (int i = 0; i < 32; i++) {
        __m512i temp = _mm512_set1_epi32(x1 ^ x2 ^ x3 ^ round_keys[i]);
        temp = sm4_sbox_vprold(temp);
        temp = sm4_linear_transform_vprold(temp);
        
        uint32_t result = _mm512_extract_epi32(temp, 0);
        uint32_t new_x = x0 ^ result;
        
        // 轮换
        x0 = x1;
        x1 = x2;
        x2 = x3;
        x3 = new_x;
    }
    
    // 反序变换并输出
    ciphertext[0] = (x3 >> 24) & 0xFF;
    ciphertext[1] = (x3 >> 16) & 0xFF;
    ciphertext[2] = (x3 >> 8) & 0xFF;
    ciphertext[3] = x3 & 0xFF;
    
    ciphertext[4] = (x2 >> 24) & 0xFF;
    ciphertext[5] = (x2 >> 16) & 0xFF;
    ciphertext[6] = (x2 >> 8) & 0xFF;
    ciphertext[7] = x2 & 0xFF;
    
    ciphertext[8] = (x1 >> 24) & 0xFF;
    ciphertext[9] = (x1 >> 16) & 0xFF;
    ciphertext[10] = (x1 >> 8) & 0xFF;
    ciphertext[11] = x1 & 0xFF;
    
    ciphertext[12] = (x0 >> 24) & 0xFF;
    ciphertext[13] = (x0 >> 16) & 0xFF;
    ciphertext[14] = (x0 >> 8) & 0xFF;
    ciphertext[15] = x0 & 0xFF;
    
    return 0;
}

// VPROLD优化的单块解密
int sm4_decrypt_block_vprold(const uint8_t *ciphertext, uint8_t *plaintext, 
                            const uint32_t *round_keys) {
    if (!check_vprold_support()) {
        return -1;
    }
    
    if (!ciphertext || !plaintext || !round_keys) {
        return -2;
    }
    
    // 加载密文
    uint32_t x0 = (ciphertext[0] << 24) | (ciphertext[1] << 16) | 
                  (ciphertext[2] << 8) | ciphertext[3];
    uint32_t x1 = (ciphertext[4] << 24) | (ciphertext[5] << 16) | 
                  (ciphertext[6] << 8) | ciphertext[7];
    uint32_t x2 = (ciphertext[8] << 24) | (ciphertext[9] << 16) | 
                  (ciphertext[10] << 8) | ciphertext[11];
    uint32_t x3 = (ciphertext[12] << 24) | (ciphertext[13] << 16) | 
                  (ciphertext[14] << 8) | ciphertext[15];
    
    // 32轮逆向迭代
    for (int i = 31; i >= 0; i--) {
        __m512i temp = _mm512_set1_epi32(x1 ^ x2 ^ x3 ^ round_keys[i]);
        temp = sm4_sbox_vprold(temp);
        temp = sm4_linear_transform_vprold(temp);
        
        uint32_t result = _mm512_extract_epi32(temp, 0);
        uint32_t new_x = x0 ^ result;
        
        // 轮换
        x0 = x1;
        x1 = x2;
        x2 = x3;
        x3 = new_x;
    }
    
    // 反序变换并输出
    plaintext[0] = (x3 >> 24) & 0xFF;
    plaintext[1] = (x3 >> 16) & 0xFF;
    plaintext[2] = (x3 >> 8) & 0xFF;
    plaintext[3] = x3 & 0xFF;
    
    plaintext[4] = (x2 >> 24) & 0xFF;
    plaintext[5] = (x2 >> 16) & 0xFF;
    plaintext[6] = (x2 >> 8) & 0xFF;
    plaintext[7] = x2 & 0xFF;
    
    plaintext[8] = (x1 >> 24) & 0xFF;
    plaintext[9] = (x1 >> 16) & 0xFF;
    plaintext[10] = (x1 >> 8) & 0xFF;
    plaintext[11] = x1 & 0xFF;
    
    plaintext[12] = (x0 >> 24) & 0xFF;
    plaintext[13] = (x0 >> 16) & 0xFF;
    plaintext[14] = (x0 >> 8) & 0xFF;
    plaintext[15] = x0 & 0xFF;
    
    return 0;
}

// VPROLD优化的16块并行加密
int sm4_encrypt_16blocks_vprold(const uint8_t *plaintext, uint8_t *ciphertext, 
                               const uint32_t *round_keys) {
    if (!check_vprold_support()) {
        return -1;
    }
    
    if (!plaintext || !ciphertext || !round_keys) {
        return -2;
    }
    
    // 加载16个块的数据
    __m512i x0 = _mm512_setzero_si512();
    __m512i x1 = _mm512_setzero_si512();
    __m512i x2 = _mm512_setzero_si512();
    __m512i x3 = _mm512_setzero_si512();
    
    // 转换字节序并加载
    for (int block = 0; block < 16; block++) {
        const uint8_t *src = plaintext + block * 16;
        uint32_t w0 = (src[0] << 24) | (src[1] << 16) | (src[2] << 8) | src[3];
        uint32_t w1 = (src[4] << 24) | (src[5] << 16) | (src[6] << 8) | src[7];
        uint32_t w2 = (src[8] << 24) | (src[9] << 16) | (src[10] << 8) | src[11];
        uint32_t w3 = (src[12] << 24) | (src[13] << 16) | (src[14] << 8) | src[15];
        
        x0 = _mm512_insert_epi32(x0, w0, block);
        x1 = _mm512_insert_epi32(x1, w1, block);
        x2 = _mm512_insert_epi32(x2, w2, block);
        x3 = _mm512_insert_epi32(x3, w3, block);
    }
    
    // 32轮并行处理
    for (int round = 0; round < 32; round++) {
        __m512i temp = _mm512_xor_si512(x1, x2);
        temp = _mm512_xor_si512(temp, x3);
        temp = _mm512_xor_si512(temp, _mm512_set1_epi32(round_keys[round]));
        
        // S盒变换
        temp = sm4_sbox_vprold(temp);
        
        // 线性变换
        temp = sm4_linear_transform_vprold(temp);
        
        // 更新状态
        __m512i new_x = _mm512_xor_si512(x0, temp);
        x0 = x1;
        x1 = x2;
        x2 = x3;
        x3 = new_x;
    }
    
    // 反序变换并输出
    for (int block = 0; block < 16; block++) {
        uint8_t *dst = ciphertext + block * 16;
        uint32_t w0 = _mm512_extract_epi32(x3, block);
        uint32_t w1 = _mm512_extract_epi32(x2, block);
        uint32_t w2 = _mm512_extract_epi32(x1, block);
        uint32_t w3 = _mm512_extract_epi32(x0, block);
        
        dst[0] = (w0 >> 24) & 0xFF; dst[1] = (w0 >> 16) & 0xFF;
        dst[2] = (w0 >> 8) & 0xFF;  dst[3] = w0 & 0xFF;
        dst[4] = (w1 >> 24) & 0xFF; dst[5] = (w1 >> 16) & 0xFF;
        dst[6] = (w1 >> 8) & 0xFF;  dst[7] = w1 & 0xFF;
        dst[8] = (w2 >> 24) & 0xFF; dst[9] = (w2 >> 16) & 0xFF;
        dst[10] = (w2 >> 8) & 0xFF; dst[11] = w2 & 0xFF;
        dst[12] = (w3 >> 24) & 0xFF; dst[13] = (w3 >> 16) & 0xFF;
        dst[14] = (w3 >> 8) & 0xFF; dst[15] = w3 & 0xFF;
    }
    
    return 0;
}

// VPROLD优化的ECB模式加密
int sm4_ecb_encrypt_vprold(const uint8_t *plaintext, uint8_t *ciphertext, 
                          size_t length, const uint32_t *round_keys) {
    if (!check_vprold_support()) {
        return -1;
    }
    
    if (!plaintext || !ciphertext || !round_keys || length % 16 != 0) {
        return -2;
    }
    
    size_t blocks = length / 16;
    size_t offset = 0;
    
    // 16块并行处理
    while (blocks >= 16) {
        sm4_encrypt_16blocks_vprold(plaintext + offset, ciphertext + offset, round_keys);
        offset += 256;  // 16 * 16 bytes
        blocks -= 16;
    }
    
    // 处理剩余块
    while (blocks > 0) {
        sm4_encrypt_block_vprold(plaintext + offset, ciphertext + offset, round_keys);
        offset += 16;
        blocks--;
    }
    
    return 0;
}

// 检查VPROLD指令可用性
int sm4_vprold_available(void) {
    return check_vprold_support();
}

// 获取VPROLD优化信息
const char* sm4_vprold_info(void) {
    if (check_vprold_support()) {
        return "VPROLD (Vector Packed Rotate Left Double-word) optimization enabled";
    } else {
        return "VPROLD optimization not available";
    }
}
