# SM4 加密算法优化项目

## 项目概述

本项目专注于 SM4 分组密码算法的性能优化实现，包括多种优化技术的研究与实现。

## 项目结构

```
sm4-optimization/
├── src/ # 源代码目录
│ ├── basic/ # 基础实现
│ ├── ttable/ # T表优化实现
│ ├── simd/ # SIMD指令优化实现
│ └── common/ # 公共模块
├── tests/ # 测试代码
├── benchmarks/ # 性能测试
├── docs/ # 文档
└── README.md
```

## 开发计划

### 阶段 1: 基础实现
- [x] 创建项目结构
- [ ] 实现基本 SM4 算法
- [ ] 编写单元测试
- [ ] 性能基准测试

### 阶段 2: T表优化
- [ ] 实现 T表优化版本
- [ ] 性能对比分析
- [ ] 文档记录

### 阶段 3: SIMD优化
- [ ] 实现 SIMD 指令优化
- [ ] 性能对比分析
- [ ] 文档记录

## 编译和运行

### 环境要求
- GCC 9.0+ 或 Clang 10.0+
- 支持 AVX2/AVX512 指令集（用于 SIMD 优化）

### 编译
```bash
make all
```

### 运行测试
```bash
make test
```

### 性能测试
```bash
make benchmark
```

## 分支策略

我们采用功能分支工作流：
- `main`: 主分支，包含稳定版本
- `feature/basic-implementation`: 基础实现
- `feature/ttable-optimization`: T表优化
- `feature/simd-optimization`: SIMD优化

每个特性在独立分支开发，完成后合并到主分支。
