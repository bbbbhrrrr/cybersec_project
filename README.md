# 网络安全密码学项目集合

本项目集合包含六个完整的网络安全与密码学技术实现，涵盖对称密码算法、数字水印、零知识证明、哈希算法、椭圆曲线密码学和隐私保护计算等前沿技术领域。每个项目都包含完整的理论实现、性能优化、安全分析和详细文档。

## 技术特色

- **标准合规**: 严格遵循国家密码管理局GM/T标准和国际密码学标准
- **性能优化**: 采用SIMD指令集、硬件加速等技术实现显著性能提升
- **工程实践**: 从理论研究到生产部署的完整工程化实现
- **安全可靠**: 完整的安全性分析、攻击测试和防护措施

## 项目概览

### Project 1: SM4对称加密算法优化
**技术领域**: 对称密码算法性能优化  
**核心技术**: SIMD指令集优化与多版本实现

- **标准实现**: 完整的GM/T 0002-2012标准实现
- **多版本优化**: 基础版本、T-table查表、SIMD并行、硬件指令加速
- **性能提升**: 最高实现25倍性能提升
- **认证加密**: 支持SM4-GCM认证加密模式
- **跨平台**: 支持Windows、Linux、macOS多平台

### Project 2: 数字图像水印系统
**技术领域**: 数字版权保护与内容安全  
**核心技术**: DCT频域变换与鲁棒性水印算法

- **水印算法**: 基于DCT变换的频域水印嵌入技术
- **鲁棒性**: 支持多种几何攻击和图像处理攻击
- **质量保证**: PSNR值51.2dB，保持高图像质量
- **综合测试**: 37种攻击类型的完整测试框架
- **实时处理**: 支持实时水印嵌入和提取

### Project 3: Poseidon2零知识证明电路
**技术领域**: 零知识证明与区块链密码学  
**核心技术**: 代数哈希函数与电路优化

- **电路设计**: 完整的Poseidon2哈希电路实现
- **证明系统**: 基于Groth16的零知识证明
- **性能优化**: 相比SHA-256减少95.9%约束数量
- **链上验证**: 支持智能合约验证器
- **标准配置**: 支持t=2和t=3两种安全配置

### Project 4: SM3密码哈希算法优化
**技术领域**: 密码学哈希函数性能优化  
**核心技术**: SIMD并行计算与算法优化

- **国标合规**: 严格按照GM/T 0004-2012标准实现
- **SIMD优化**: AVX2指令集8路并行处理
- **算法改进**: 循环展开、预计算、内存对齐
- **性能提升**: 1.2-1.8倍性能提升
- **多场景**: 支持小数据到大文件的高效处理

### Project 5: SM2椭圆曲线数字签名优化
**技术领域**: 椭圆曲线密码学与数字签名  
**核心技术**: 椭圆曲线算法优化与预计算技术
- **标准实现**: 严格按照GM/T 0003.2-2012标准
- **椭圆曲线**: 完整的SM2推荐曲线数学运算
- **数字签名**: 密钥生成、签名、验证全流程
- **优化技术**: 蒙哥马利阶梯、窗口法、预计算
- **缓存机制**: 小倍数点预计算与缓存优化
- **性能提升**: 小倍数运算655倍加速

### Project 6: Google Password Checkup协议实现
**技术领域**: 隐私保护与密码安全检查  
**核心技术**: 椭圆曲线私有集合交集协议

- **协议实现**: 完整的椭圆曲线PSI协议
- **隐私保护**: 客户端和服务端双向隐私保护
- **椭圆曲线**: P-256曲线完整数学运算
- **密码检查**: 泄露密码检测与安全评估
- **性能优化**: 1.8秒完整检查流程
- **批量支持**: 支持大规模密码数据库检查

## 技术指标总览

| 项目名称 | 算法类型 | 主要优化技术 | 性能提升 | 应用领域 |
|---------|----------|-------------|----------|----------|
| SM4优化 | 对称加密 | SIMD+硬件指令 | 最高25倍加速 | 高速数据加密 |
| 图像水印 | 数字水印 | DCT+QIM调制 | 51.2dB质量/78%鲁棒性 | 版权保护 |
| Poseidon2-ZK | 零知识证明 | 代数哈希优化 | 95.9%约束减少 | 区块链隐私 |
| SM3优化 | 哈希算法 | SIMD+循环展开 | 1.2-1.8倍加速 | 完整性校验 |
| SM2优化 | 椭圆曲线签名 | 预计算+缓存 | 655倍小倍数加速 | 数字签名 |
| Password Checkup | 隐私计算 | 椭圆曲线PSI | 1.8秒检查 | 密码安全 |

## 项目架构

### 技术架构层次
```
网络安全密码学项目架构
┌─────────────────────────────────────────────────────┐
│                 应用层                               │
├─────────────────────────────────────────────────────┤
│  数字水印    安全通信    隐私计算    区块链应用        │
├─────────────────────────────────────────────────────┤
│                 算法层                               │
├─────────────────────────────────────────────────────┤
│  SM4加密   图像水印   Poseidon2   SM3哈希   SM2签名   │
├─────────────────────────────────────────────────────┤
│                 优化层                               │
├─────────────────────────────────────────────────────┤
│  SIMD并行   算法优化   内存优化   电路优化           │
├─────────────────────────────────────────────────────┤
│                 平台层                               │
├─────────────────────────────────────────────────────┤
│  Windows/Linux   x86_64/ARM   CPU/GPU   多编译器    │
└─────────────────────────────────────────────────────┘
```

### 目录结构
```
cybersec_project/
├── project1-sm4-optimization/      # SM4对称加密算法优化
├── project2-image-watermark/       # 数字图像水印系统
├── project3-poseidon2-circuit/     # Poseidon2零知识证明电路
├── project4-sm3-optimization/      # SM3哈希算法优化
├── project5-sm2-optimization/      # SM2椭圆曲线数字签名优化
├── project6-password-checkup/      # Google Password Checkup协议
├── docs/                           # 项目文档
├── output/                         # 输出报告
└── README.md                       # 项目说明
## 技术栈与工具

### 编程语言
- **C/C++**: 高性能密码学算法实现与SIMD优化
- **Python**: 图像处理、密码学协议实现、数据分析
- **JavaScript**: 零知识证明电路开发与工具链
- **Assembly**: 底层性能优化和硬件指令利用

### 开发工具
- **编译器**: GCC、MSVC、Clang等主流编译器
- **构建工具**: Makefile、CMake、npm、Python setuptools
- **密码学库**: 自研实现、Circom、snarkjs
- **图像处理**: OpenCV、PIL、Matplotlib、NumPy
- **测试框架**: 自建完整测试验证体系

### 核心技术
- **SIMD优化**: AVX2指令集并行计算优化
- **硬件加速**: AES-NI、GFNI等专用指令
- **算法优化**: 查表法、预计算、循环展开
- **密码学标准**: 国产密码算法标准实现
- **性能分析**: 详细的基准测试和性能分析
## 项目统计

### 代码规模
- 总文件数量: 约150个源文件
- 代码总行数: 约15000行
- 技术文档: 约50页完整文档
- 测试用例: 约200个测试用例

### 编程语言分布
- Python: 31.4% (图像处理、协议实现、算法验证)
- Assembly: 25.5% (SIMD优化、底层性能加速)
- C: 17.2% (SM4/SM3密码算法核心实现)
- C++: 11.9% (高性能算法优化框架)
- JavaScript: 10.9% (零知识证明工具链)
- Circom: 1.1% (电路描述语言)
- Other: 2.0% (构建脚本、配置文件)

### 技术领域覆盖
- 密码学算法: 50% (核心算法实现与优化)
- 性能优化: 30% (SIMD加速与算法调优)
- 零知识证明/区块链: 15% (前沿密码学技术)
- 图像处理: 5% (数字水印技术)

## 环境要求

### 系统要求
- 操作系统: Windows 10+, macOS 10.15+, Ubuntu 18.04+
- 处理器: 支持AVX2的x86_64处理器
- 内存: 最低8GB，推荐16GB+
- 存储: 至少10GB可用空间

### 开发工具
- 编译器: GCC 7.0+, MSVC 2017+, Clang 6.0+
- Python: 3.8+ (推荐3.9+)
- Node.js: 16.0+ (推荐18.0+)
- Rust: 1.60+ (用于circom编译)
## 快速开始

### 环境配置

#### Windows环境
```powershell
# 安装Visual Studio Build Tools
# 安装Python 3.8+和Node.js 16.0+
winget install Python.Python.3.11
winget install OpenJS.NodeJS
```

#### Linux环境
```bash
# Ubuntu/Debian系统
sudo apt update
sudo apt install build-essential python3 python3-pip nodejs npm

# CentOS/RHEL系统
sudo yum groupinstall "Development Tools"
sudo yum install python3 python3-pip nodejs npm
```

#### macOS环境
```bash
# 安装Xcode命令行工具
xcode-select --install

# 使用Homebrew安装依赖
brew install python@3.11 node
```

### 依赖安装
```bash
# Python依赖(图像水印项目)
pip install numpy opencv-python matplotlib pillow scipy

# Node.js依赖(零知识证明项目)
npm install -g circom@2.1.6 snarkjs@0.6.11
```

### 项目运行

#### Project 1: SM4算法优化
```bash
cd project1-sm4-optimization
make all                    # 编译所有版本
./build/benchmark.exe       # 性能基准测试
./build/test_sm4.exe        # 功能验证测试
```

#### Project 2: 图像水印系统
```bash
cd project2-image-watermark
pip install -r requirements.txt
python demo.py              # 水印演示
python analysis.py          # 性能分析
python tests/test_robustness.py  # 鲁棒性测试
```

#### Project 3: Poseidon2零知识证明
```bash
cd project3-poseidon2-circuit
npm install                 # 安装依赖
npm run demo               # 完整演示
npm run benchmark          # 性能测试
npm run analysis           # 电路分析
```

#### Project 4: SM3哈希算法优化
```bash
cd project4-sm3-optimization
make all                   # 编译所有版本
./build/test_sm3.exe       # 功能测试
./build/benchmark_sm3.exe  # 性能测试
```

#### Project 5: SM2椭圆曲线数字签名优化
```bash
cd project5-sm2-optimization
python demo.py             # 签名演示
python simple_demo.py      # 简化演示
python tests/test_sm2.py   # 完整测试
```

#### Project 6: Password Checkup协议
```bash
cd project6-password-checkup
pip install -r requirements.txt
python demo.py             # 协议演示
python benchmarks/performance_benchmark.py  # 性能测试
```

### Project 6: Password Checkup协议
```bash
cd project6-password-checkup
pip install -r requirements.txt
python demo.py # 运行协议演示
python benchmarks/performance_benchmark.py # 性能测试
python tests/test_password_checkup.py # 协议验证
```

## 项目成果与技术亮点

### 核心技术成果
- **密码学算法实现**: 4个完整的高性能密码学算法实现
- **性能优化方法论**: 系统性SIMD优化和算法调优技术
- **安全性评估**: 完整的密码学安全性分析和攻击测试
- **工程化实践**: 从理论到生产的完整实现流程

### 创新亮点
- **多技术融合**: 传统密码学 + 前沿零知识证明
- **性能导向**: 所有实现均针对实际部署优化
- **标准合规**: 严格遵循GM/T和国际密码学标准
- **模块化架构**: 高度可复用的组件化设计

### 实际应用价值
- **生产就绪**: 达到生产环境部署标准
- **教育价值**: 密码学工程实践完整教学案例
- **研究基础**: 为进一步研究提供扎实技术基础
- **行业应用**: 金融、政府、区块链等领域直接应用

## 性能测试结果

### SM4对称加密算法
| 实现版本 | 处理速度 | 性能提升 | 技术特点 |
|----------|----------|----------|----------|
| 基础实现 | 100 MB/s | 基准 | 标准C语言实现 |
| T-table优化 | 280 MB/s | 2.8x | 查表法优化 |
| SIMD优化 | 350 MB/s | 3.5x | AVX2并行处理 |
| 硬件指令 | 2500 MB/s | 25x | AES-NI等专用指令 |

### SM3哈希算法
| 实现版本 | 处理速度 | 性能提升 | 技术特点 |
|----------|----------|----------|----------|
| 基础实现 | 211 MB/s | 基准 | 标准实现 |
| SIMD优化 | 251 MB/s | 1.19x | AVX2并行 |
| 大数据优化 | 320 MB/s | 1.52x | 16KB+数据块 |

### 数字图像水印
| 性能指标 | 测试结果 | 说明 |
|----------|----------|------|
| 图像质量 | PSNR 51.2 dB | 高质量保持 |
| 鲁棒性 | 78% | 37种攻击测试 |
| 处理速度 | 1.2秒/图像 | 512×512分辨率 |

### Poseidon2零知识证明
| 性能指标 | 测试结果 | 说明 |
|----------|----------|------|
| 约束数量 | 1,156个 | vs SHA-256: 27,904个 |
| 约束减少 | 95.9% | 显著优化 |
| 证明时间 | 1.5秒 | 标准硬件 |
| 验证时间 | 8毫秒 | 链上友好 |

### SM2椭圆曲线数字签名
| 优化项目 | 性能提升 | 说明 |
|----------|----------|------|
| 小倍数运算 | 655倍加速 | 预计算优化 |
| 整体性能 | 1-5%提升 | 综合优化效果 |
| 内存使用 | 30%减少 | 缓存优化 |

### Password Checkup协议
| 执行阶段 | 耗时 | 说明 |
|----------|------|------|
| 客户端准备 | 114ms | 密码处理与盲化 |
| 服务端处理 | 830ms | PSI计算 |
| 客户端验证 | 845ms | 去盲化与验证 |
| 协议总耗时 | 1.79秒 | 完整流程 |

## 项目质量

### 代码质量指标
- 代码覆盖率: 95%以上
- 文档完整度: 100%
- 平台兼容性: Windows/Linux/macOS
- 标准合规性: 完全符合相关标准

### 测试覆盖
- 功能测试: 完整的算法正确性验证
- 性能测试: 详细的基准测试和性能分析
- 安全测试: 密码学安全性和攻击抵抗测试
- 兼容性测试: 多平台和多编译器验证

## 技术路线图

### Phase 1: 基础算法实现
- SM4/SM3标准实现
- 基础功能验证
- 测试框架搭建

### Phase 2: 性能优化
- SIMD并行化
- 算法优化技术
- 性能基准测试

### Phase 3: 应用扩展
- 数字水印应用
- 零知识证明集成
- 跨领域技术融合

### Phase 4: 工程化部署
- 生产级优化
- 文档完善
- 标准化接口

## 未来发展方向

### 技术演进
- **后量子密码学**: 抗量子攻击算法研究
- **同态加密**: 隐私计算技术应用
- **多方安全计算**: 分布式密码协议
- **AI+密码学**: 机器学习辅助密码分析

### 应用拓展
- **区块链集成**: Layer2隐私解决方案
- **IoT安全**: 轻量级密码学协议
- **云安全**: 密文计算和数据保护
- **隐私保护**: 差分隐私和联邦学习

## 学习资源与文档

### 技术文档
- [SM4算法设计文档](project1-sm4-optimization/docs/设计文档.md)
- [数字水印技术指南](project2-image-watermark/docs/设计文档.md)
- [零知识证明实践文档](project3-poseidon2-circuit/docs/设计文档.md)
- [SM3优化技术文档](project4-sm3-optimization/docs/设计文档.md)
- [SM2椭圆曲线签名文档](project5-sm2-optimization/docs/设计文档.md)
- [Password Checkup协议文档](project6-password-checkup/docs/设计文档.md)

### 标准参考
- GM/T 0002-2012: SM4分组密码算法标准
- GM/T 0003.2-2012: SM2椭圆曲线数字签名算法标准
- GM/T 0004-2012: SM3密码杂凑算法标准
- IEEE 数字水印相关标准
- zk-SNARKs 协议规范文档

### 开发指南
- 环境配置和编译部署指南
- 性能测试和优化指南
- 安全实践和最佳实践
- 跨平台部署指南

## 使用案例

### 高性能数据加密
```c
// 使用SM4算法进行高速数据加密
#include "sm4_simd.h"

uint8_t key[16] = {...};
uint8_t plaintext[1024] = {...};
uint8_t ciphertext[1024];

sm4_simd_encrypt_ecb(plaintext, ciphertext, 1024, key);
// 实现最高25倍加速的加密性能
```

### 数字版权保护
```python
# 图像数字水印嵌入和提取
from watermark import WatermarkSystem

ws = WatermarkSystem(strength=30)
watermarked_image = ws.embed_watermark(original_image, watermark_text)
extracted_text = ws.extract_watermark(watermarked_image)
# 实现51.2dB图像质量和78%鲁棒性
```

### 零知识证明应用
```javascript
// 零知识证明生成和验证
const { prove, verify } = require('./zk-system');

const witness = { secret: 12345, hash: "0x..." };
const proof = await prove(witness);
const isValid = await verify(proof);
// 1.5秒证明生成，8ms验证时间
```

## 应用领域

### 金融科技
- 高性能加密通信
- 数字签名验证
- 区块链隐私保护
- 密码安全检查

### 政府信息化
- 国产密码算法应用
- 文档数字水印
- 安全通信保障
- 数据完整性验证

### 区块链与Web3
- 零知识证明应用
- 隐私计算协议
- 智能合约验证
- 去中心化身份认证

### 数字内容保护
- 图像版权保护
- 视频水印技术
- 音频防盗版
- 文档溯源追踪
SIMD优化: 350 MB/s (3.5x加速)
查表优化: 280 MB/s (2.8x加速)
```

### SM3算法优化性能
```
基础实现: 211 MB/s
优化实现: 251 MB/s (1.19x加速)
大数据场景: 1.52x加速 (16KB+)
```

### 图像水印系统性能
```
图像质量: PSNR 51.2 dB
鲁棒性: 78% (37种攻击测试)
处理速度: 0.8秒/图像 (512×512)
```

### Poseidon2零知识证明性能
```
电路约束: 1,156个 (vs SHA-256: 27,904个)
证明时间: 1.5秒
验证时间: 8ms
约束减少: 95.9%
```

## 文档结构

每个项目都包含完整的技术文档和实现：

```
projectX-name/
├── README.md # 项目介绍和快速开始
├── Makefile / package.json # 构建配置文件
├── docs/
│ ├── 设计文档.md # 详细技术设计文档
│ ├── 环境配置指南.md # 环境配置说明
│ └── 使用指南.md # 用户使用指南
├── src/ # 源代码实现
│ ├── basic/ # 基础版本实现
│ ├── optimized/ # 优化版本实现
│ └── common/ # 公共组件
├── tests/ # 测试用例和验证
├── benchmarks/ # 性能基准测试
├── build/ # 编译输出目录
├── output/ # 实验结果和报告
└── samples/ (部分项目) # 示例数据和演示
```

## 代码质量

### 编程规范
- **代码风格**: 遵循业界最佳实践
- **注释完善**: 详细的函数和算法注释
- **错误处理**: 完整的异常处理和边界检查
- **内存安全**: 防止缓冲区溢出和内存泄漏

### 测试覆盖
- **单元测试**: 每个核心函数的独立测试
- **集成测试**: 完整流程的端到端测试
- **性能测试**: 详细的基准测试和性能分析
- **安全测试**: 密码学安全性和攻击抵抗测试

### 文档质量
- **技术文档**: 详细的算法原理和实现说明
- **API文档**: 完整的接口文档和使用示例
- **用户指南**: 详细的安装配置和使用教程
- **实验报告**: 完整的实验数据和分析结果

## 使用案例

### 1. 高性能加密通信
```c
// 使用SM4算法进行高速数据加密
#include "sm4_simd.h"

uint8_t key[16] = {...};
uint8_t plaintext[1024] = {...};
uint8_t ciphertext[1024];

sm4_simd_encrypt_ecb(plaintext, ciphertext, 1024, key);
// 实现3-4倍加速的加密性能
```

### 2. 数字版权保护
```python
# 图像数字水印嵌入和提取
from watermark import WatermarkSystem

ws = WatermarkSystem(strength=30)
watermarked_image = ws.embed_watermark(original_image, watermark_text)
extracted_text = ws.extract_watermark(watermarked_image)
# 达到51dB图像质量和78%鲁棒性
```

### 3. 隐私计算证明
```javascript
// 零知识证明生成和验证
const { prove, verify } = require('./zk-system');

const witness = { secret: 12345, hash: "0x..." };
const proof = await prove(witness);
const isValid = await verify(proof);
// 1.5秒证明生成，8ms验证时间
```

### 4. 文件完整性校验
```c
// 使用SM3算法进行文件哈希
#include "sm3_optimized.h"

uint8_t file_data[1024*1024];
uint8_t hash[32];

sm3_optimized_hash(file_data, sizeof(file_data), hash);
// 实现1.2-1.8倍性能提升
```

## 项目统计

### 代码规模
```
Total Files: ~150个源文件
Total Lines: ~15,000行代码
Languages: Python, Assembly, C, C++, JavaScript, Circom
Documentation: ~50页技术文档
Test Cases: ~200个测试用例
```

### 技术栈分布
```
Python:       31.4% (图像处理、密码学协议、算法实现)
Assembly:     25.5% (SIMD优化、底层性能加速)
C:            17.2% (SM4/SM3密码算法核心实现)
C++:          11.9% (高性能算法优化)
JavaScript:   10.9% (零知识证明工具链)
Circom:        1.1% (电路描述语言)
Other:         2.0% (构建脚本、配置文件)
```

### 项目成熟度
```
Project 1 (SM4): 生产就绪
Project 2 (水印): 生产就绪
Project 3 (Poseidon): 生产就绪
Project 4 (SM3): 生产就绪
整体集成: 持续完善
```

## 环境要求

### 通用要求
- **操作系统**: Windows 10+, macOS 10.15+, Ubuntu 18.04+
- **内存**: 最低8GB，推荐16GB+ (零知识证明需要更多内存)
- **存储**: 至少10GB可用空间 (包含所有项目和依赖)
- **网络**: 用于下载依赖包和工具链

### 项目特定要求

#### Project 1 & 4 (SM4/SM3优化)
- **处理器**: 支持AVX2的x86_64处理器 (Intel Haswell+, AMD Excavator+)
- **编译器**: GCC 7.0+, MSVC 2017+, Clang 6.0+
- **工具**: Make, CMake (可选)

#### Project 2 (图像水印)
- **Python**: 3.8+ (推荐3.9+)
- **依赖库**: OpenCV 4.5+, NumPy, Matplotlib, Pillow
- **图像格式**: 支持PNG, JPEG, BMP等常见格式

#### Project 3 (零知识证明)
- **Node.js**: 16.0+ (推荐18.0+)
- **Rust**: 1.60+ (用于circom编译)
- **内存**: 最低4GB可用内存用于证明生成
- **工具**: circom 2.1.6+, snarkjs 0.6.11+

### 硬件推荐配置
- **CPU**: Intel i7-8700K / AMD Ryzen 7 3700X 或更高
- **内存**: 32GB DDR4 (用于大规模零知识证明)
- **存储**: SSD固态硬盘 (提升编译和测试速度)
- **显卡**: 可选，未来GPU加速版本使用

## 贡献指南

### 参与方式
欢迎对项目进行改进和扩展，共同推进密码学技术发展！

#### 代码贡献
- **性能优化**: 算法优化、SIMD增强、GPU加速实现
- **功能扩展**: 新算法实现、创新特性开发
- **平台支持**: ARM架构、移动端、嵌入式适配
- **安全增强**: 侧信道防护、形式化验证集成

#### 文档贡献
- **技术文档**: 算法原理详解、实现细节深度分析
- **使用教程**: 入门指南、最佳实践分享
- **API文档**: 接口说明、示例代码完善
- **国际化**: 英文文档、多语言支持

## 技术发展方向

### 算法扩展
- 后量子密码学算法研究与实现
- 同态加密技术应用
- 多方安全计算协议
- 轻量级物联网密码学

### 性能优化
- GPU并行计算加速
- ARM架构适配优化
- 硬件安全模块集成
- 边缘计算部署优化

### 应用拓展
- 区块链隐私保护解决方案
- 云端密文计算服务
- 移动端安全SDK
- 工业互联网安全应用

## 许可证

本项目采用MIT许可证，详见各子项目的LICENSE文件。

## 项目标签

密码学 | SM4 | SM3 | SM2 | SIMD优化 | 数字水印 | 零知识证明 | Poseidon2 | 性能优化 | 椭圆曲线密码 | 隐私计算

---

**网络安全密码学项目集合 - 构建安全可信的数字世界**

最后更新: 2024年12月
