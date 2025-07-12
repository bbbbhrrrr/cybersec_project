# Project 3: Poseidon2 哈希算法零知识证明电路

## 项目概述

本项目使用Circom实现Poseidon2哈希算法的零知识证明电路，采用Groth16算法生成证明。该电路允许证明者在不泄露原象的情况下证明知道某个哈希值的原象。

## 技术规格

### Poseidon2 参数
- **字段大小 (n)**: 256位
- **状态大小 (t)**: 3（备选：2）
- **S-box 度数 (d)**: 5
- **参考**: [Poseidon2论文](https://eprint.iacr.org/2023/323.pdf) Table 1

### 电路设计
- **公开输入**: Poseidon2哈希值
- **隐私输入**: 哈希原象（单个block）
- **证明系统**: Groth16

## 项目结构

```
project3-poseidon2-circuit/
├── circuits/ # Circom电路文件
│ ├── poseidon2.circom # 主电路实现
│ └── utils/ # 辅助电路模块
├── contracts/ # 智能合约验证器
├── scripts/ # 构建和测试脚本
├── tests/ # 测试用例
├── docs/ # 项目文档
├── build/ # 编译输出
└── setup/ # 可信设置文件
```

## 快速开始

### 环境要求
- Node.js >= 16.0.0
- Circom >= 2.1.6
- snarkjs >= 0.7.0

### 安装依赖
```bash
npm install
```

### 编译电路
```bash
npm run compile
```

### 生成证明
```bash
npm run prove
```

### 验证证明
```bash
npm run verify
```

## 技术特性

- **标准兼容**: 严格按照Poseidon2规范实现
- **参数优化**: 使用推荐的安全参数
- **模块化设计**: 可复用的电路组件
- **完整测试**: 全面的功能和安全测试
- **Groth16证明**: 高效的零知识证明生成

## 许可证

MIT License
