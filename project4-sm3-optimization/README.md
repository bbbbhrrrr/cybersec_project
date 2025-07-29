# Project 4: SM3 Hash Algorithm Optimization

## 项目概述

本项目实现了中国国家密码标准SM3哈希算法的多种优化版本，包括基础实现、SIMD优化、高级优化技术，以及基于SM3的安全分析和实际应用。项目参考付勇老师的PPT，系统性地展示了从基础实现到高性能优化的完整过程。

## 项目特性

### 多种实现版本
- **基础实现**: 严格遵循GM/T 0004-2012标准的参考实现
- **SIMD优化**: 利用AVX2指令集实现4路并行处理
- **高级优化**: 包含循环展开、缓存优化、寄存器调度等技术

### 安全分析
- **Length-Extension攻击**: 演示SM3算法的长度扩展攻击漏洞
- **攻击验证**: 完整的攻击实现和验证过程
- **安全建议**: 针对实际应用的安全使用指导

### 实际应用
- **Merkle树**: 基于RFC6962标准的Certificate Transparency实现
- **大规模支持**: 支持10万级叶子节点的树结构
- **证明系统**: 完整的存在性和不存在性证明

### 性能优化
- **多级优化**: 从基础到高级的渐进式优化
- **性能分析**: 详细的性能测试和对比分析
- **跨平台**: 支持Windows、Linux、macOS多平台

## 技术架构

```
project4-sm3-optimization/
├── src/                    # 源代码目录
│   ├── common/            # 公共头文件和定义
│   │   └── sm3_common.h   # SM3核心数据结构和API
│   ├── basic/             # 基础实现
│   │   └── sm3_basic.c    # 标准SM3实现
│   ├── simd/              # SIMD优化实现
│   │   └── sm3_simd.c     # AVX2并行优化
│   ├── optimized/         # 高级优化实现
│   │   └── sm3_optimized.c # 循环展开和缓存优化
│   ├── security/          # 安全分析工具
│   │   └── sm3_length_extension.c # 长度扩展攻击
│   └── applications/      # 实际应用
│       └── sm3_merkle_tree.c # Merkle树实现
├── tests/                 # 测试代码
│   └── test_sm3_comprehensive.c # 综合测试
├── benchmarks/            # 性能测试
│   └── benchmark_comprehensive.c # 性能基准测试
├── docs/                  # 文档目录
│   └── 设计文档.md        # 详细设计文档
├── build/                 # 构建输出目录
├── output/                # 测试输出目录
├── build.sh              # Linux/macOS构建脚本
├── build.bat             # Windows构建脚本
├── Makefile              # Make构建文件
└── README.md             # 本文件
```

## 快速开始

### 环境要求

#### 最低要求
- C99兼容的编译器 (GCC 4.8+, Clang 3.9+, MSVC 2015+)
- CMake 3.10+ (可选)
- Make工具

#### 推荐配置
- 支持AVX2指令集的现代CPU (Intel Haswell+, AMD Zen+)
- GCC 8.0+ 或 Clang 8.0+ (更好的优化支持)
- 8GB+ RAM (用于大规模Merkle树测试)

### 编译安装

#### Linux/macOS (推荐)
```bash
# 克隆仓库
git clone <repository-url>
cd project4-sm3-optimization

# 给构建脚本执行权限
chmod +x build.sh

# 编译所有组件 (发布模式)
./build.sh

# 编译并运行测试
./build.sh --test

# 编译并运行性能测试
./build.sh --benchmark

# 调试模式编译
./build.sh --debug

# 查看更多选项
./build.sh --help
```

#### Windows
```cmd
# 进入项目目录
cd project4-sm3-optimization

# 编译所有组件 (需要MSVC或MinGW)
build.bat

# 编译并运行测试
build.bat /test

# 编译并运行性能测试
build.bat /benchmark

# 调试模式编译
build.bat /debug

# 查看更多选项
build.bat /help
```

#### 使用Makefile
```bash
# 编译所有组件
make all

# 仅编译基础实现
make basic

# 仅编译SIMD实现
make simd

# 运行测试
make test

# 运行性能测试
make benchmark

# 清理构建文件
make clean
```

### 使用示例

#### 1. 基础哈希计算
```c
#include "src/common/sm3_common.h"

uint8_t data[] = "Hello, SM3!";
uint8_t hash[SM3_DIGEST_SIZE];

// 计算哈希值
sm3_hash(data, strlen((char*)data), hash);

// 打印结果
for (int i = 0; i < SM3_DIGEST_SIZE; i++) {
    printf("%02x", hash[i]);
}
printf("\n");
```

#### 2. 长度扩展攻击演示
```bash
# 运行长度扩展攻击工具
./build/sm3_length_extension

# 查看攻击详细过程和结果
```

#### 3. Merkle树操作
```bash
# 运行Merkle树演示
./build/sm3_merkle_tree

# 构建10万节点的树并生成证明
```

#### 4. 性能测试
```bash
# 运行综合性能测试
./build/benchmark_comprehensive

# 查看不同实现的性能对比
```

## 性能数据

### 测试环境
- **CPU**: Intel Core i7-10700K @ 3.8GHz
- **内存**: 32GB DDR4-3200
- **编译器**: GCC 11.2.0
- **编译选项**: -O3 -march=native

### 性能对比 (MB/s)

| 数据大小 | 基础实现 | 优化实现 | SIMD实现 | 提升倍数 |
|----------|----------|----------|----------|----------|
| 64B      | 145.2    | 198.7    | 387.3    | 2.67x    |
| 1KB      | 162.8    | 234.5    | 445.2    | 2.73x    |
| 64KB     | 178.3    | 267.9    | 512.6    | 2.87x    |
| 1MB      | 182.1    | 275.4    | 534.8    | 2.94x    |

*注：实际性能会因硬件配置和系统负载而变化*

### 内存使用

| 组件 | 最小内存 | 典型使用 | 峰值使用 |
|------|----------|----------|----------|
| 基础实现 | 256B | 1KB | 4KB |
| Merkle树(10万节点) | 25MB | 35MB | 50MB |
| 批处理SIMD | 1KB | 16KB | 64KB |

## 安全特性

### Length-Extension攻击
本项目实现了对SM3算法的长度扩展攻击演示，用于教育和安全研究目的：

1. **攻击原理**: 利用Merkle-Damgård构造的特性
2. **攻击实现**: 完整的状态恢复和消息扩展
3. **防护措施**: HMAC-SM3等安全构造方案

### 安全建议
- 在需要认证的场景中使用HMAC-SM3
- 避免直接使用哈希值作为MAC
- 实施适当的输入验证和长度检查

## API文档

### 核心函数

#### `sm3_hash()`
```c
void sm3_hash(const uint8_t *data, size_t len, uint8_t *digest);
```
计算输入数据的SM3哈希值。

**参数**:
- `data`: 输入数据指针
- `len`: 输入数据长度
- `digest`: 输出缓冲区，必须至少32字节

#### `sm3_init()`, `sm3_update()`, `sm3_final()`
```c
void sm3_init(sm3_ctx_t *ctx);
void sm3_update(sm3_ctx_t *ctx, const uint8_t *data, size_t len);
void sm3_final(sm3_ctx_t *ctx, uint8_t *digest);
```
流式哈希计算接口。

### SIMD接口

#### `sm3_simd_hash_batch()`
```c
void sm3_simd_hash_batch(const uint8_t *data[], const size_t lengths[], 
                         uint8_t digests[][SM3_DIGEST_SIZE], uint32_t count);
```
批量并行哈希计算（需要AVX2支持）。

### Merkle树接口

#### `merkle_tree_build()`
```c
merkle_tree_t* merkle_tree_build(const uint8_t *leaves[], uint32_t count);
```
构建Merkle树。

#### `merkle_tree_prove_inclusion()`
```c
merkle_proof_t* merkle_tree_prove_inclusion(merkle_tree_t *tree, uint32_t index);
```
生成包含性证明。

## 测试说明

### 测试向量
项目包含官方测试向量和自定义测试用例：
- GM/T 0004-2012标准测试向量
- 空字符串、单字节、多块数据测试
- 随机数据一致性测试
- 性能回归测试

### 运行测试
```bash
# 运行所有测试
./build/test_sm3_comprehensive

# 运行特定测试类别
./build/test_sm3_comprehensive --basic
./build/test_sm3_comprehensive --simd
./build/test_sm3_comprehensive --security
```
