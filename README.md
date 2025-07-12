# 网络空间安全创新创业实践课程项目

## 项目概述

本仓库包含网络空间安全创新创业实践课程的项目作业，采用模块化的项目管理方式，每个项目独立开发和维护。

## 项目列表

### Project 1: SM4 加密算法性能优化
- **文件夹**: `project1-sm4-optimization/`
- **描述**: 实现SM4分组密码算法的多种性能优化技术，包括基础实现、T表优化和批处理优化
- **技术栈**: C语言、GCC编译器、AVX2指令集
- **状态**: ✅ 已完成
- **性能提升**: T表优化+46%，批处理优化+18%

### Project 2: 图片数字水印系统
- **文件夹**: `project2-image-watermark/`
- **描述**: 基于DCT频域变换的数字图像水印系统，支持水印嵌入、提取和鲁棒性测试
- **技术栈**: Python、OpenCV、NumPy、PIL
- **状态**: ✅ 已完成
- **特色功能**: 37种攻击测试、完整评估框架、自动化演示

## 项目结构

```
cybersec_project/
├── project1-sm4-optimization/      # 项目1：SM4算法优化
│   ├── src/                        # 源代码
│   ├── tests/                      # 测试代码
│   ├── benchmarks/                 # 性能测试
│   ├── docs/                       # 项目文档
│   └── README.md                   # 项目说明
├── project2-image-watermark/       # 项目2：图片数字水印
│   ├── src/                        # 源代码
│   ├── tests/                      # 测试代码
│   ├── samples/                    # 测试图片
│   ├── output/                     # 输出结果
│   ├── docs/                       # 项目文档
│   └── README.md                   # 项目说明
└── README.md                       # 总体说明
```

## 开发环境

- **操作系统**: Windows 11 x64
- **处理器**: AMD Ryzen 5 6600H with Radeon Graphics
- **内存**: 16GB DDR4
- **编译器**: GCC 8.1.0 (MinGW-W64)
- **开发工具**: Visual Studio Code + Git

## 联系信息

- **开发者**: bbbbhrrrr
- **邮箱**: 1376203696@qq.com
- **GitHub**: https://github.com/bbbbhrrrr/cybersec_project

## 更新日志

### 2025-07-12
- 初始化Git仓库
- 完成Project 1: SM4算法优化项目
- 完成Project 2: 图片数字水印系统  
- 重新整理项目结构，添加项目编号
- 建立完整的开发和测试框架
