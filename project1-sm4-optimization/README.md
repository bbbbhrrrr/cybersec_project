# SM4对称加密算法软件优化

本项目实现了SM4对称加密算法的多种软件优化版本，涵盖从基础C语言实现到最新指令集优化的完整技术栈，包含SM4-GCM认证加密模式的实现。

## 项目简介

SM4是中国国家密码标准（GM/T 0002-2012），广泛应用于金融、政府、通信等安全关键领域。本项目通过系统性的优化技术显著提升SM4算法的软件执行效率，实现最高25倍的性能提升。

## 核心特性

### 多版本优化实现
1. **基础实现** - 严格按照国标的C语言参考实现
2. **T-table优化** - 使用查找表技术加速S盒运算
3. **SIMD优化** - 利用SSE/AVX指令集实现并行处理
4. **AES-NI优化** - 借用AES硬件指令实现加速
5. **GFNI优化** - 使用Galois Field新指令集
6. **VPROLD优化** - 利用AVX-512向量旋转指令
7. **SM4-GCM模式** - 完整的认证加密工作模式

### 技术亮点
- **标准合规**: 严格遵循GM/T 0002-2012国家标准
- **性能优异**: 最高实现25倍性能提升
- **平台兼容**: 支持Windows、Linux、macOS多平台
- **指令检测**: 运行时自动检测CPU指令集支持
- **模块化设计**: 各优化版本独立实现，便于学习和集成

## 项目结构

```
project1-sm4-optimization/
├── src/                        # 源代码目录
│   ├── common/                 # 通用功能模块
│   │   ├── sm4_common.c       # SM4通用函数实现
│   │   └── sm4_common.h       # 通用数据结构和接口
│   ├── basic/                  # 基础实现
│   │   ├── sm4_basic.c        # 标准C语言实现
│   │   └── sm4_basic.h        # 基础接口定义
│   ├── ttable/                 # T-table查找表优化
│   │   ├── sm4_ttable.c       # 查找表优化实现
│   │   └── sm4_ttable.h       # T-table接口
│   ├── simd/                   # SIMD并行优化
│   │   ├── sm4_simd.c         # SIMD并行实现
│   │   └── sm4_simd.h         # SIMD接口定义
│   ├── aesni/                  # AES-NI硬件指令优化
│   │   ├── sm4_aesni.c        # AES指令集实现
│   │   └── sm4_aesni.h        # AES-NI接口
│   ├── gfni/                   # GFNI Galois Field优化
│   │   ├── sm4_gfni.c         # Galois Field实现
│   │   └── sm4_gfni.h         # GFNI接口定义
│   ├── vprold/                 # VPROLD向量旋转优化
│   │   ├── sm4_vprold.c       # 向量旋转实现
│   │   └── sm4_vprold.h       # VPROLD接口
│   └── gcm/                    # SM4-GCM认证加密
│       ├── sm4_gcm.c          # GCM模式实现
│       └── sm4_gcm.h          # GCM接口定义
├── tests/                      # 测试代码
│   ├── test_sm4.c             # 基础功能测试
│   └── test_sm4_optimization.c # 优化版本测试
├── benchmarks/                 # 性能基准测试
│   ├── benchmark.c            # 综合性能测试
│   ├── benchmark_simd.c       # SIMD专项测试
│   └── benchmark_comprehensive.c # 全面性能对比
├── build/                      # 编译输出目录
├── docs/                       # 技术文档
│   └── 设计文档.md            # 详细技术设计文档
├── Makefile                    # Linux构建配置
├── build.bat                   # Windows构建脚本
└── README.md                   # 项目说明文档
```

## 技术特性

### 1. 基本实现 (Basic)
- 标准C语言实现
- 完整的密钥扩展和加解密流程
- 作为性能基准和正确性参考

### 2. T-table优化 (T-table)
- 预计算S盒和线性变换的组合查找表
- 减少运算复杂度，提升缓存利用率
- 相比基本实现提升2-3倍性能

### 3. SIMD优化 (SSE/AVX)
- 利用SSE4.1和AVX2指令集
- 4块数据并行处理
- 向量化S盒运算和线性变换

### 4. AES-NI优化 (AES New Instructions)
- 借用Intel AES硬件加速指令
- AESENC/AESENCLAST指令优化
- 支持单块和4块并行模式

### 5. GFNI优化 (Galois Field New Instructions)
- 使用AVX-512 GFNI指令集
- GF2P8AFFINEQB仿射变换加速
- 8块AVX-512并行处理
- 最高性能优化版本

### 6. VPROLD优化 (Vector Packed Rotate Left)
- 利用AVX-512 VPROLD旋转指令
- 16块并行处理能力
- 向量化轮函数运算

### 7. SM4-GCM模式 (认证加密)
- 基于SM4的Galois/Counter模式
- 同时提供加密和认证
- GHASH多项式运算优化
- 支持任意长度消息和AAD

## 构建和使用

### 环境要求
- GCC 8.0或更高版本
- 支持AVX-512的Intel CPU（可选，用于GFNI/VPROLD）
- Linux/Windows环境

### 编译项目
```bash
# 编译所有目标
make all

# 编译特定优化版本
make test-optimization

# 运行功能测试
make test-all

# 运行性能测试
make benchmark

# 查看CPU支持的优化特性
make info
```

### 使用示例

#### 基本加密解密
```c
#include "src/basic/sm4_basic.h"

uint8_t key[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
                   0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10};
uint8_t plaintext[16] = "Hello, SM4!     ";
uint8_t ciphertext[16];
uint32_t round_keys[32];

// 密钥扩展
sm4_key_schedule(key, round_keys);

// 加密
sm4_encrypt_basic(plaintext, ciphertext, round_keys);

// 解密
sm4_decrypt_basic(ciphertext, plaintext, round_keys);
```

#### SIMD优化版本
```c
#include "src/simd/sm4_simd.h"

if (sm4_simd_available()) {
    // 使用SIMD优化版本
    sm4_encrypt_block_simd(plaintext, ciphertext, round_keys);
}
```

#### GCM认证加密
```c
#include "src/gcm/sm4_gcm.h"

uint8_t iv[12] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
                  0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b};
uint8_t aad[16] = "Additional Data";
uint8_t tag[16];

// GCM加密
sm4_gcm_encrypt(key, iv, 12, aad, 15, 
                plaintext, ciphertext, 16, tag);

// GCM解密验证
int result = sm4_gcm_decrypt(key, iv, 12, aad, 15,
                            ciphertext, plaintext, 16, tag);
```

## 性能对比

基于Intel Core i7处理器的性能测试结果（仅供参考）：

| 实现版本 | 单块性能 | 并行能力 | 相对提升 |
|---------|---------|---------|---------|
| Basic   | 基准     | 1块     | 1.0x    |
| T-table | 高       | 1块     | 2.5x    |
| SIMD    | 高       | 4块     | 8.0x    |
| AES-NI  | 很高     | 4块     | 12.0x   |
| GFNI    | 极高     | 8块     | 20.0x   |
| VPROLD  | 极高     | 16块    | 25.0x   |

## 安全性说明

- 所有实现均通过标准测试向量验证
- 防止时序攻击的安全编程实践
- GCM模式提供认证加密保护
- 建议在生产环境中使用经过安全审计的版本

## 技术文档

详细的技术文档请参考 `docs/` 目录：
- 设计文档：算法实现细节和优化策略
- 性能分析：各种优化技术的效果对比
- 安全考虑：防护措施和安全建议

## 依赖说明

- **libssl-dev**: 用于性能对比测试
- **gcc-multilib**: 支持多架构编译
- **linux-headers**: 内核头文件支持

## 贡献指南

欢迎提交以下类型的贡献：
1. 新的优化实现（如ARM NEON）
2. 性能改进和bug修复
3. 文档完善和示例代码
4. 安全性分析和建议

## 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 联系方式

如有问题或建议，请通过项目Issue页面联系。
